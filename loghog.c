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

#define BUFFER_SIZE 4096

// ANSI color codes for dark theme
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31;1m"
#define ANSI_COLOR_GREEN "\x1b[32;1m"
#define ANSI_COLOR_LIGHT_GREEN "\x1b[92;1m"
#define ANSI_COLOR_YELLOW "\x1b[33;1m"
#define ANSI_COLOR_BLUE "\x1b[34;1m"
#define ANSI_COLOR_MAGENTA "\x1b[35;1m"
#define ANSI_COLOR_CYAN "\x1b[36;1m"
#define ANSI_COLOR_WHITE "\x1b[37;1m"
#define ANSI_COLOR_LIGHT_GRAY "\x1b[37;1m"
#define ANSI_COLOR_DARK "\x1b[30m"
#define ANSI_COLOR_BG "\x1b[48;5;235m"

#define ASCII_ART "\n" \
ANSI_COLOR_MAGENTA " 888                       888    888  .d88888b.   .d8888b. " ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888                       888    888 d88P\" \"Y88b d88P  Y88b" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888                       888    888 888     888 888    888" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888      .d88b.   .d88b.  8888888888 888     888 888       " ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888     d88\"\"88b d88P\"88b 888    888 888     888 888  88888" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888     888  888 888  888 888    888 888     888 888    888" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888     Y88..88P Y88b 888 888    888 Y88b. .d88P Y88b  d88P" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 88888888 \"Y88P\"   \"Y88888 888    888  \"Y88888P\"   \"Y8888P88" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA "                       888" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA "                  Y8b d88P " ANSI_COLOR_LIGHT_GRAY " The" ANSI_COLOR_LIGHT_GREEN " NO " ANSI_COLOR_LIGHT_GRAY "Nonsense Log Search Tool" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA "                   \"Y88P\"" ANSI_COLOR_RESET "\n"

// Menu option definitions
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

// Function prototypes
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

// Global search paths
char log_search_path[BUFFER_SIZE] = "/var/lib/docker /var/log";

/**
 * Creates a find command to search for log files in the specified path.
 * @param buffer Output buffer to store the generated find command.
 * @param size Size of the output buffer.
 * @param search_path Path(s) to search logs in.
 */
void find_logs_command(char *buffer, size_t size, const char *search_path) {
    snprintf(buffer, size, "find %s -type f \\( -name '*.log' -o -name 'messages' -o -name 'cron' -o -name 'maillog' -o -name 'secure' -o -name 'firewalld' \\) -exec tail -f -n +1 {} +", search_path);
}

/**
 * Displays a text buffer in the 'less' pager for convenient viewing.
 * @param buffer The text buffer to display.
 * @param length Length of the text buffer.
 */
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

/**
 * Runs a shell command and processes the output using a specified action function.
 * @param cmd The command to run.
 * @param buffer_action Function to process the command output.
 */
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

/**
 * Searches logs for authentication-related issues and displays them using 'less'.
 * @param log_search_path Paths to search for logs.
 */
void live_auth_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"authentication(\\s*failed)?|permission(\\s*denied)?|invalid\\s*(user|password|token)|(unauthorized|illegal)\\s*(access|attempt)|SQL\\s*injection|cross-site\\s*(scripting|request\\s*Forgery)|directory\\s*traversal|(brute-?force|DoS|DDoS)\\s*attack|(vulnerability|exploit)\\s*(detected|scan)\"", find_cmd);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

/**
 * Searches logs for error-related issues and displays them using 'less'.
 * @param log_search_path Paths to search for logs.
 */
void live_error_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"\\b(?:error|fail(?:ed|ure)?|warn(?:ing)?|critical|socket|denied|refused|retry|reset|timeout|dns|network)\"", find_cmd);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

/**
 * Displays all logs using 'less'.
 * @param log_search_path Paths to search for logs.
 */
void live_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s", find_cmd);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

/**
 * Searches logs for network protocol-related issues and displays them using 'less'.
 * @param log_search_path Paths to search for logs.
 */
void live_network_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i 'https?://|ftps?://|telnet://|ssh://|sftp://|ldap(s)?://|nfs://|tftp://|gopher://|imap(s)?://|pop3(s)?://|smtp(s)?://|rtsp://|rtmp://|mms://|xmpp://|ipp://|xrdp://'", find_cmd);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

/**
 * Prompts user for input and returns the input string.
 * @param prompt Prompt message to display.
 * @return The user input string.
 */
char *get_user_input(const char *prompt) {
    char *input = readline(prompt);
    if (input && *input) {
        add_history(input);
    }
    return input;
}

/**
 * Validates the user input to ensure it is within bounds.
 * @param input Input string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int sanitize_input(char *input) {
    if (input == NULL || strlen(input) == 0) {
        return 0;
    }

    // Ensure the input does not exceed the buffer size
    if (strlen(input) >= BUFFER_SIZE) {
        printf(ANSI_COLOR_RED "Input too long. Please try again.\n" ANSI_COLOR_RESET);
        return 0;
    }

    return 1;
}

/**
 * Prompts the user to enter a regular expression and searches the logs.
 * @param log_search_path Paths to search for logs.
 */
void run_regex(const char *log_search_path) {
    char *egrep_args = get_user_input("\nRegEX > ");
    if (!sanitize_input(egrep_args)) {
        free(egrep_args);
        return;
    }

    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"%s\"", find_cmd, egrep_args);
    run_command_with_buffer(cmd, display_buffer_with_less);
    free(egrep_args);
}

/**
 * Prompts the user to enter an IP or regex pattern and searches the logs.
 * @param log_search_path Paths to search for logs.
 */
void search_ip(const char *log_search_path) {
    char *ip_regex = get_user_input("\nIP / RegEX > ");
    if (!sanitize_input(ip_regex)) {
        free(ip_regex);
        return;
    }

    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"%s\"", find_cmd, ip_regex);
    run_command_with_buffer(cmd, display_buffer_with_less);
    free(ip_regex);
}

/**
 * Allows the user to edit the log search paths.
 * @param log_search_path Paths to search for logs.
 */
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

/**
 * Prompts the user to enter a regex or pattern and exports the results to a JSON file.
 * @param log_search_path Paths to search for logs.
 */
void export_search_results_to_json(const char *log_search_path) {
    char *egrep_args = get_user_input("\nRegEX / Text > ");
    if (!sanitize_input(egrep_args)) {
        free(egrep_args);
        return;
    }

    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=never -i \"%s\"", find_cmd, egrep_args);

    FILE *proc = popen(cmd, "r");
    if (!proc) {
        perror("popen");
        free(egrep_args);
        return;
    }

    json_object *json_arr = json_object_new_array();
    char buffer[BUFFER_SIZE];
    int has_entries = 0;
    char all_entries[BUFFER_SIZE * 10] = "";

    while (fgets(buffer, sizeof(buffer), proc)) {
        json_object *json_obj = json_object_new_object();
        json_object_object_add(json_obj, "log_entry", json_object_new_string(buffer));
        json_object_array_add(json_arr, json_obj);

        strcat(all_entries, buffer);
        has_entries = 1;
    }

    pclose(proc);

    if (has_entries) {
        char output_file[BUFFER_SIZE] = "log_search_results.json";
        FILE *output = fopen(output_file, "w");
        if (output) {
            fputs(json_object_to_json_string(json_arr), output);
            fclose(output);
            printf("\nExported search results to %s\n", output_file);
        } else {
            perror("fopen");
        }

        // Print the results to the screen using `less`
        display_buffer_with_less(all_entries, strlen(all_entries));
    } else {
        printf("\nNo matching log entries found.\n");
    }

    json_object_put(json_arr);
    free(egrep_args);
}

/**
 * Displays the help information for the tool.
 */
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
        ANSI_COLOR_BLUE "real time log analysisi " ANSI_COLOR_CYAN "(h for help)" ANSI_COLOR_BLUE " or " ANSI_COLOR_CYAN "(q to quit)\n" ANSI_COLOR_RESET
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
        ANSI_COLOR_BLUE "     '\\bREGEX_PATTERN\\b'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '\\b(?:[0-9]{1,3}\\.){3}[0-9]{1,3}\\b'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(authentication|permission|invalid user)'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(DoS|DDoS attack)'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'brute-?force|directory traversal'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(SQL injection|cross-site scripting)'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '(GET|POST|PUT|DELETE|PATCH) /[a-zA-Z0-9/_-]*'\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     'cron.*\\((root|admin)\\)'\n\n" ANSI_COLOR_RESET
        ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "I" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "P (Log Search)\n" ANSI_COLOR_RESET
        ANSI_COLOR_CYAN " - Filters logs by ip, ranges, and regular expressions.\n" ANSI_COLOR_RESET
        ANSI_COLOR_BLUE "     '\\b(?:[0-9]{1,3}\\.){3}[0-9]{1,3}\\b'\n" ANSI_COLOR_RESET
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

/**
 * Displays the main menu and handles user input.
 */
void main_menu() {
    char *option;
    while (1) {
        printf(ANSI_COLOR_GREEN ASCII_ART ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "A" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "uth\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "E" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "rrors\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "L" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "ive All\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "N" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "etwork\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "R" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "egex\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "I" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "P Search\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "S" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "et Log Paths\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "J" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "SON Export\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "H" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "elp\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "[" ANSI_COLOR_LIGHT_GREEN "Q" ANSI_COLOR_DARK "]" ANSI_COLOR_BLUE "uit\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "\n-" ANSI_COLOR_LIGHT_GRAY "> " ANSI_COLOR_RESET);

        option = readline(NULL);
        if (option == NULL) {
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        char opt = option[0];
        switch (opt) {
            case 'A':
            case 'a':
                live_auth_log(log_search_path);
                break;
            case 'E':
            case 'e':
                live_error_log(log_search_path);
                break;
            case 'L':
            case 'l':
                live_log(log_search_path);
                break;
            case 'N':
            case 'n':
                live_network_log(log_search_path);
                break;
            case 'R':
            case 'r':
                run_regex(log_search_path);
                break;
            case 'I':
            case 'i':
                search_ip(log_search_path);
                break;
            case 'S':
            case 's':
                edit_log_paths(log_search_path);
                break;
            case 'J':
            case 'j':
                export_search_results_to_json(log_search_path);
                break;
            case 'H':
            case 'h':
                display_help();
                break;
            case 'Q':
            case 'q':
                free(option);
                exit(0);
                break;
            default:
                printf(ANSI_COLOR_RED "Invalid option. Please try again.\n" ANSI_COLOR_RESET);
        }
        free(option);
    }
}

/**
 * Handles the SIGINT signal to return to the main menu.
 * @param sig Signal number.
 */
void sigint_handler(int sig) {
    printf("\nReturning to menu...\n");
    fflush(stdout);
}

/**
 * Main entry point of the application.
 * @return Exit status code.
 */
int main() {
    signal(SIGINT, sigint_handler);
    main_menu();
    return 0;
}
