#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <json-c/json.h>
#include <readline/readline.h>
#include <readline/history.h>

#define BUFFER_SIZE 4096  // Buffer size for command strings and general usage

// ANSI color codes for enhanced terminal output
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31;1m"
#define ANSI_COLOR_GREEN "\x1b[32;1m"
#define ANSI_COLOR_YELLOW "\x1b[33;1m"
#define ANSI_COLOR_BLUE "\x1b[34;1m"
#define ANSI_COLOR_MAGENTA "\x1b[35;1m"
#define ANSI_COLOR_CYAN "\x1b[36;1m"
#define ANSI_COLOR_WHITE "\x1b[37;1m"
#define ANSI_COLOR_LIGHT_GRAY "\x1b[37;1m"
#define ANSI_COLOR_DARK "\x1b[30m"

// ASCII art for the application header
#define ASCII_ART \
    ANSI_COLOR_MAGENTA "888                       888    888  .d88888b.   .d8888b. " ANSI_COLOR_RESET "\n" \
    ANSI_COLOR_MAGENTA "888                       888    888 d88P\" \"Y88b d88P  Y88b" ANSI_COLOR_RESET "\n" \
    ANSI_COLOR_MAGENTA "888                       888    888 888     888 888    888" ANSI_COLOR_RESET "\n" \
    ANSI_COLOR_MAGENTA "888      .d88b.   .d88b.  8888888888 888     888 888       " ANSI_COLOR_RESET "\n" \
    ANSI_COLOR_MAGENTA "888     d88\"\"88b d88P\"88b 888    888 888     888 888  88888" ANSI_COLOR_RESET "\n" \
    ANSI_COLOR_MAGENTA "888     888  888 888  888 888    888 888     888 888    888" ANSI_COLOR_RESET "\n" \
    ANSI_COLOR_MAGENTA "888     Y88..88P Y88b 888 888    888 Y88b. .d88P Y88b  d88P" ANSI_COLOR_RESET "\n" \
    ANSI_COLOR_MAGENTA "88888888 \"Y88P\"   \"Y88888 888    888  \"Y88888P\"   \"Y8888P88" ANSI_COLOR_RESET "\n" \
    ANSI_COLOR_MAGENTA "                       888" ANSI_COLOR_RESET "\n" \
    ANSI_COLOR_MAGENTA "                  Y8b d88P " ANSI_COLOR_RED " NO" ANSI_COLOR_LIGHT_GRAY "-nonsense digital forensics" ANSI_COLOR_RESET "\n" \
    ANSI_COLOR_MAGENTA "                   \"Y88P\"" ANSI_COLOR_RESET "\n"

typedef enum {
    MENU_LIVE_AUTH_LOG = 1,
    MENU_LIVE_ERROR_LOG,
    MENU_LIVE_LOG,
    MENU_LIVE_NETWORK_LOG,
    MENU_RUN_REGEX,
    MENU_SEARCH_IP,
    MENU_EDIT_LOG_PATHS,
    MENU_EXPORT_JSON,
    MENU_HELP,
    MENU_EXIT
} MenuOptions;

void find_logs_command(char *buffer, size_t size, const char *search_path);
void display_buffer_with_less(const char *buffer, size_t length);
void run_command_with_buffer(const char *cmd, void (*buffer_action)(const char *, size_t));
void live_auth_log(const char *log_search_path);
void live_error_log(const char *log_search_path);
void live_log(const char *log_search_path);
void live_network_log(const char *log_search_path);
void run_regex(const char *log_search_path);
void search_ip(const char *log_search_path);
void edit_log_paths(char *log_search_path);
void export_search_results_to_json(const char *log_search_path);
void display_help();
void main_menu();
void sigint_handler(int sig);

char log_search_path[BUFFER_SIZE] = "/var/log";

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
    char *egrep_args = get_user_input("Enter regex pattern: ");
    if (!sanitize_input(egrep_args)) {
        printf("Invalid input.\n");
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
    char *ip_regex = get_user_input("Enter IP or regex to search: ");
    if (!sanitize_input(ip_regex)) {
        printf("Invalid input.\n");
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
    printf("Current log paths: %s\n", log_search_path);
    char *new_paths = get_user_input("Enter new log paths (separated by spaces): ");
    if (!sanitize_input(new_paths)) {
        printf("Invalid input.\n");
        free(new_paths);
        return;
    }

    strncpy(log_search_path, new_paths, BUFFER_SIZE - 1);
    log_search_path[BUFFER_SIZE - 1] = '\0';
    printf("Updated log paths: %s\n", log_search_path);
    free(new_paths);
}

void export_search_results_to_json(const char *log_search_path) {
    char *egrep_args = get_user_input("Enter regex pattern for export: ");
    if (!sanitize_input(egrep_args)) {
        printf("Invalid input.\n");
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
            printf("Exported search results to %s\n", output_file);
        } else {
            perror("fopen");
        }
    } else {
        printf("No matching log entries found.\n");
    }
    json_object_put(json_arr);
    free(egrep_args);
}

void display_help() {
    printf(
        "Help Menu:\n"
        "  a) Auth Log - Displays authentication related logs.\n"
        "  e) Error Log - Displays logs related to errors.\n"
        "  l) Live Log - Displays all logs in real-time.\n"
        "  n) Network Log - Displays logs related to network activity.\n"
        "  r) Run Regex - Applies a regex filter on the logs.\n"
        "  s) Search IP - Filters logs by IP address or regex.\n"
        "  p) Edit Paths - Changes the directories searched for logs.\n"
        "  j) Export JSON - Exports log search results to a JSON file.\n"
        "  h) Help - Displays this help menu.\n"
        "  q) Quit - Exits the program.\n"
    );
}

void main_menu() {
    char *option;
    printf(ASCII_ART);
    while (true) {
        printf("\nMain Menu:\n"
               "  a) Auth Log\n"
               "  e) Error Log\n"
               "  l) Live Log\n"
               "  n) Network Log\n"
               "  r) Run Regex\n"
               "  s) Search IP\n"
               "  p) Edit Paths\n"
               "  j) Export JSON\n"
               "  h) Help\n"
               "  q) Quit\n");
        option = readline("Select an option: ");
        if (!option) continue;

        switch (option[0]) {
            case 'a': live_auth_log(log_search_path); break;
            case 'e': live_error_log(log_search_path); break;
            case 'l': live_log(log_search_path); break;
            case 'n': live_network_log(log_search_path); break;
            case 'r': {
                char *regex = readline("Enter regex pattern: ");
                if (sanitize_input(regex)) {
                    run_regex(log_search_path, regex);
                    free(regex);
                }
                break;
            }
            case 's': {
                char *ip = readline("Enter IP or regex to search: ");
                if (sanitize_input(ip)) {
                    search_ip(log_search_path, ip);
                    free(ip);
                }
                break;
            }
            case 'p': edit_log_paths(log_search_path); break;
            case 'j': export_search_results_to_json(log_search_path); break;
            case 'h': display_help(); break;
            case 'q': free(option); exit(0);
            default: printf("Invalid option. Please try again.\n");
        }
        free(option);
    }
}

void sigint_handler(int sig) {
    printf("\nInterrupt received, shutting down...\n");
    exit(sig);
}

int main(int argc, char **argv) {
    // Setup signal handling
    signal(SIGINT, sigint_handler);

    // Start the main menu
    main_menu();

    return 0;
}
