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
        "Help text goes here with detailed descriptions of each menu option.\n"
        ANSI_COLOR_LIGHT_GRAY "\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\n" ANSI_COLOR_RESET;
    display_buffer_with_less(help_text, strlen(help_text));
}
