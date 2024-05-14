#include "loghog_functions.h"
#include "ansi_colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <json-c/json.h>
#include <readline/readline.h>
#include <readline/history.h>

void find_logs_command(char *buffer, size_t size, const char *search_path) {
    snprintf(buffer, size, "find %s -type f \\( -name '*.log' -o -name 'messages' -o -name 'cron' -o -name 'maillog' -o -name 'secure' -o -name 'firewalld' \\) -exec tail -f -n +1 {} +", search_path);
}

void display_buffer_with_less(const char *buffer, size_t length) {
    char tmp_filename[] = "/tmp/logsearchXXXXXX";
    int tmp_fd = mkstemp(tmp_filename);
    if (tmp_fd == -1) {
        perror("mkstemp");
        return;
    }

    FILE *tmp_file = fdopen(tmp_fd, "w+");
    if (tmp_file == NULL) {
        perror("fdopen");
        close(tmp_fd);
        return;
    }

    fwrite(buffer, 1, length, tmp_file);
    fflush(tmp_file);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "less -R %s", tmp_filename);
    system(cmd);

    fclose(tmp_file);
    remove(tmp_filename);
}

void run_command_with_buffer(const char *cmd, void (*buffer_action)(const char *, size_t)) {
    FILE *proc = popen(cmd, "r");
    if (!proc) {
        perror("popen");
        return;
    }

    char *output = NULL;
    size_t total_length = 0;
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), proc)) {
        size_t buffer_length = strlen(buffer);
        char *temp = realloc(output, total_length + buffer_length + 1);
        if (!temp) {
            perror("realloc");
            free(output);
            pclose(proc);
            return;
        }
        output = temp;
        memcpy(output + total_length, buffer, buffer_length);
        total_length += buffer_length;
        output[total_length] = '\0';

        fputs(buffer, stdout);
        fflush(stdout);
    }

    if (buffer_action) {
        buffer_action(output, total_length);
    }

    free(output);
    pclose(proc);
}

void live_auth_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"authentication(\\s*failed)?|permission(\\s*denied)?|invalid\\s*(user|password|token)|(unauthorized|illegal)\\s*(access|attempt)|SQL\\s*injection|cross-site\\s*(scripting|request\\s*Forgery)|directory\\s*traversal|(brute-?force|DoS|DDoS)\\s*attack|(vulnerability|exploit)\\s*(detected|scan)\"", find_cmd);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

void live_error_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"\\b(?:error|fail(?:ed|ure)?|warn(?:ing)?|critical|socket|denied|refused|retry|reset|timeout|dns|network)\"", find_cmd);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

void live_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s", find_cmd);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

void live_network_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i 'https?://|ftps?://|telnet://|ssh://|sftp://|ldap(s)?://|nfs://|tftp://|gopher://|imap(s)?://|pop3(s)?://|smtp(s)?://|rtsp://|rtmp://|mms://|xmpp://|ipp://|xrdp://'", find_cmd);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

void run_regex(const char *log_search_path) {
    char *egrep_args = get_user_input("\nRegEX > ");
    if (!sanitize_input(egrep_args)) {
        free(egrep_args);
        return;
    }

    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"%s\"", find_cmd, egrep_args);
    run_command_with_buffer(cmd, display_buffer_with_less);
    free(egrep_args);
}

void search_ip(const char *log_search_path) {
    char *ip_regex = get_user_input("\nIP / RegEX > ");
    if (!sanitize_input(ip_regex)) {
        free(ip_regex);
        return;
    }

    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"%s\"", find_cmd, ip_regex);
    run_command_with_buffer(cmd, display_buffer_with_less);
    free(ip_regex);
}

void edit_log_paths(char *log_search_path) {
    char *new_paths = get_user_input("\nCurrent log paths: /var/lib/docker /var/log\nEnter new log paths (separated by spaces) > ");
    if (!sanitize_input(new_paths)) {
        free(new_paths);
        return;
    }

    strncpy(log_search_path, new_paths, BUFFER_SIZE - 1);
    log_search_path[BUFFER_SIZE - 1] = '\0';
    free(new_paths);
    printf(ANSI_COLOR_GREEN "Updated log paths: %s\n" ANSI_COLOR_RESET, log_search_path);
}

void export_search_results_to_json(const char *log_search_path) {
    char *egrep_args = get_user_input("\nRegEX / Text > ");
    if (!sanitize_input(egrep_args)) {
        free(egrep_args);
        return;
    }

    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=never -i \"%s\"", find_cmd, egrep_args);

    FILE *proc = popen(cmd, "r");
    if (!proc) {
        perror("popen");
        free(egrep_args);
        return;
    }

    json_object *json_arr = json_object_new_array();
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), proc)) {
        json_object *json_obj = json_object_new_object();
        json_object_object_add(json_obj, "log_entry", json_object_new_string(buffer));
        json_object_array_add(json_arr, json_obj);
    }

    pclose(proc);

    if (json_object_array_length(json_arr) > 0) {
        char output_file[] = "log_search_results.json";
        FILE *output = fopen(output_file, "w");
        if (output) {
            fputs(json_object_to_json_string_ext(json_arr, JSON_C_TO_STRING_PRETTY), output);
            fclose(output);
            printf("\nExported search results to %s\n", output_file);
        } else {
            perror("fopen");
        }
    } else {
        printf("\nNo matching log entries found.\n");
    }

    json_object_put(json_arr);
    free(egrep_args);
}

void display_help() {
    const char *help_text =
        ANSI_COLOR_LIGHT_GRAY "\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN "LogHOG " ANSI_COLOR_BLUE "is a " ANSI_COLOR_CYAN "FAST" ANSI_COLOR_BLUE " comprehensive log search tool with 2 modes\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_YELLOW "TAIL MODE\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "Logs are automatically stiched together by timestamp making \n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "events easy to follow in real time " ANSI_COLOR_CYAN "(CTRL+C to quit)\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_YELLOW "LESS MODE\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "Buffers from tail mode are sent directly to less, a powerful\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "text editing tool that allows for indepth review, searches and\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "real time log analysis " ANSI_COLOR_CYAN "(h for help)" ANSI_COLOR_BLUE " or " ANSI_COLOR_CYAN "(q to quit)\n" ANSI_COLOR_RESET
        ANSI_COLOR_LIGHT_GRAY "\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN "MENU OVERVIEW\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "A" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "uthentication (Tail)\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN " - Identify events such as.\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'authentication failed'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'permission denied'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'invalid user'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'unauthorized access'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'SQL injection detected'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'cross-site scripting attempt'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'directory traversal attack'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'and more .. '\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "E" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "rrors (Tail)\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN " - Filters logs for error-related events.\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'error'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'failure'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'critical'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'socket timeout'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'network reset'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'DNS resolution failure'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'permission denied'\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "L" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "ive (LIVE, timestamp order)\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN " - Displays every log in real time, sorted by timestamp.\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "N" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "etwork Protocol Filter (Tail)\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN " - Filters logs by protocol.\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'http://'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'https://'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'ftp://'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'ssh://'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'telnet://'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'smtp://'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'sftp://'\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "R" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "egex (Tail)\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN " - Search EVERYTHING using standard regular expressions.\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'error|failure'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'REGEX_PATTERN'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(?:[0-9]{1,3}\\.){3}[0-9]{1,3}'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(authentication|permission|invalid user)'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(DoS|DDoS attack)'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'brute-?force|directory traversal'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(SQL injection|cross-site scripting)'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(192.168.???.*) or ranges (192.168.[1..10].[1..100])\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(GET|POST|PUT|DELETE|PATCH) /[a-zA-Z0-9/_-]*'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'cron.*\\((root|admin)\\)'\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "I" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "P (Log Search)\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN " - Filters logs by IP, ranges, and regular expressions.\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(?:[0-9]{1,3}\\.){3}[0-9]{1,3}'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(192\\.168\\.[0-9]+\\.[0-9]+)'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '\\b(?:[A-Fa-f0-9]{1,4}:){7}[A-Fa-f0-9]{1,4}\\b'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(::|(?:[A-Fa-f0-9]{1,4}:){1,7}:)'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     ':(?::[A-Fa-f0-9]{1,4}){1,7}'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '192\\.168\\.\\d{1,3}\\.\\d{1,3}'\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "S" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "et (Log Paths)\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN " - Allows setting custom log paths.\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '/var/log /opt/logs'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '/var/lib/docker /var/log/nginx'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '/usr/local/logs /home/user/logs'\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "J" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "SON (Export tool)\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN " - Exports filtered logs to a JSON file in the home directory called log_search_results.json.\n" ANSI_COLOR_RESET
        ANSI_COLOR_MAGENTA "     'log_search_results.json'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'jq '.[] | .log_entry' log_search_results.json\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "H" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "elp\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN " - Displays this Help.\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "Q" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "uit\n\n" ANSI_COLOR_RESET;
    display_buffer_with_less(help_text, strlen(help_text));
}
