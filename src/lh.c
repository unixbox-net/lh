#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <json-c/json.h>
#include <readline/readline.h>
#include <readline/history.h>

#define BUFFER_SIZE 4096

// ANSI Color Definitions
#define ANSI_COLOR_RESET       "\x1b[0m"
#define ANSI_COLOR_RED         "\x1b[31m"
#define ANSI_COLOR_GREEN       "\x1b[32m"
#define ANSI_COLOR_YELLOW      "\x1b[33m"
#define ANSI_COLOR_BLUE        "\x1b[34m"
#define ANSI_COLOR_MAGENTA     "\x1b[35m"
#define ANSI_COLOR_CYAN        "\x1b[36m"
#define ANSI_COLOR_WHITE       "\x1b[37m"

typedef enum {
    MENU_LIVE_AUTH_LOG = 1,
    MENU_LIVE_ERROR_LOG,
    MENU_LIVE_NETWORK_LOG,
    MENU_RUN_REGEX,
    MENU_SEARCH_IP,
    MENU_EDIT_LOG_PATHS,
    MENU_EXPORT_JSON,
    MENU_EXIT
} MenuOptions;

// Function prototypes
void find_logs_command(char *buffer, size_t size, const char *search_path);
void display_buffer_with_less(const char *buffer);
void run_command_with_buffer(const char *cmd, void (*buffer_action)(const char *));
void live_auth_log(void);
void live_error_log(void);
void live_network_log(void);
void run_regex(void);
void search_ip(void);
void edit_log_paths(void);
void export_search_results_to_json(void);
void display_help(void);
void main_menu(void);
void sigint_handler(int sig);

char log_search_path[BUFFER_SIZE] = "/var/log";

void find_logs_command(char *buffer, const char *search_path) {
    snprintf(buffer, BUFFER_SIZE, "find %s -type f \\( -name '*.log' -o -name 'messages' -o -name 'cron' -o -name 'maillog' -o -name 'secure' -o -name 'firewalld' \\) -exec tail -f -n +1 {} +", search_path);
}

void display_buffer_with_less(const char *buffer) {
    char tmp_filename[] = "/tmp/logsearchXXXXXX";
    int tmp_fd = mkstemp(tmp_filename);
    if (tmp_fd == -1) {
        perror("mkstemp");
        return;
    }

    FILE *tmp_file = fdopen(tmp_fd, "w");
    if (!tmp_file) {
        perror("fdopen");
        close(tmp_fd);
        return;
    }

    fputs(buffer, tmp_file);
    fflush(tmp_file);
    fclose(tmp_file);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, BUFFER_SIZE, "less -R %s", tmp_filename);
    system(cmd);
    unlink(tmp_filename);
}

void run_command_with_buffer(const char *cmd, void (*buffer_action)(const char *)) {
    FILE *proc = popen(cmd, "r");
    if (!proc) {
        perror("popen");
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, proc) != NULL) {
        buffer_action(buffer);
    }

    pclose(proc);
}

char *get_user_input(const char *prompt) {
    char *input = readline(prompt);
    if (input && *input) {
        add_history(input);
    }
    return input;
}

int sanitize_input(const char *input) {
    if (input == NULL || strlen(input) == 0) {
        return 0;
    }

    if (strlen(input) >= BUFFER_SIZE) {
        fprintf(stderr, "Input too long. Please try again.\n");
        return 0;
    }

    return 1;
}

void live_auth_log(void) {
    char cmd[BUFFER_SIZE];
    find_logs_command(cmd, log_search_path);
    strncat(cmd, " | egrep --color=always -i \"authentication(\\s*failed)?|permission(\\s*denied)?|invalid\\s*(user|password|token)|(unauthorized|illegal)\\s*(access|attempt)|SQL\\s*injection|cross-site\\s*(scripting|request\\s*Forgery)|directory\\s*traversal|(brute-?force|DoS|DDoS)\\s*attack|(vulnerability|exploit)\\s*(detected|scan)\"", BUFFER_SIZE - strlen(cmd));
    run_command_with_buffer(cmd, display_buffer_with_less);
}

void live_error_log(void) {
    char cmd[BUFFER_SIZE];
    find_logs_command(cmd, log_search_path);
    strncat(cmd, " | egrep --color=always -i \"\\b(?:error|fail(?:ed|ure)?|warn(?:ing)?|critical|socket|denied|refused|retry|reset|timeout|dns|network)\"", BUFFER_SIZE - strlen(cmd));
    run_command_with_buffer(cmd, display_buffer_with_less);
}

void live_network_log(void) {
    char cmd[BUFFER_SIZE];
    find_logs_command(cmd, log_search_path);
    strncat(cmd, " | egrep --color=always -i 'https?://|ftps?://|telnet://|ssh://|sftp://|ldap(s)?://|nfs://|tftp://|gopher://|imap(s)?://|pop3(s)?://|smtp(s)?://|rtsp://|rtmp://|mms://|xmpp://|ipp://|xrdp://'", BUFFER_SIZE - strlen(cmd));
    run_command_with_buffer(cmd, display_buffer_with_less);
}

void run_regex(void) {
    char *egrep_args = get_user_input("\nRegEX > ");
    if (!sanitize_input(egrep_args)) {
        free(egrep_args);
        return;
    }

    char cmd[BUFFER_SIZE];
    find_logs_command(cmd, log_search_path);
    strncat(cmd, " | egrep --color=always -i ", BUFFER_SIZE - strlen(cmd));
    strncat(cmd, egrep_args, BUFFER_SIZE - strlen(cmd));
    run_command_with_buffer(cmd, display_buffer_with_less);
    free(egrep_args);
}

void search_ip(void) {
    char *ip_regex = get_user_input("\nIP / RegEX > ");
    if (!sanitize_input(ip_regex)) {
        free(ip_regex);
        return;
    }

    char cmd[BUFFER_SIZE];
    find_logs_command(cmd, log_search_path);
    strncat(cmd, " | egrep --color=always -i ", BUFFER_SIZE - strlen(cmd));
    strncat(cmd, ip_regex, BUFFER_SIZE - strlen(cmd));
    run_command_with_buffer(cmd, display_buffer_with_less);
    free(ip_regex);
}

void edit_log_paths(void) {
    char *new_paths = get_user_input("\nCurrent log paths: /var/lib/docker /var/log\nie: /var/lib/docker/containers /opt /nfsshare etc\n Enter new log paths (separated by spaces) > ");
    if (!sanitize_input(new_paths)) {
        free(new_paths);
        return;
    }

    strncpy(log_search_path, new_paths, BUFFER_SIZE - 1);
    log_search_path[BUFFER_SIZE - 1] = '\0';
    free(new_paths);
    printf("Updated log paths: %s\n", log_search_path);
}

void export_search_results_to_json(void) {
    char *egrep_args = get_user_input("\nRegEX / Text > ");
    if (!sanitize_input(egrep_args)) {
        free(egrep_args);
        return;
    }

    char cmd[BUFFER_SIZE];
    find_logs_command(cmd, log_search_path);
    strncat(cmd, " | egrep --color=never -i ", BUFFER_SIZE - strlen(cmd));
    strncat(cmd, egrep_args, BUFFER_SIZE - strlen(cmd));

    FILE *proc = popen(cmd, "r");
    if (proc) {
        json_object *jarray = json_object_new_array();
        char buffer[BUFFER_SIZE];

        while (fgets(buffer, sizeof(buffer), proc) != NULL) {
            json_object *jstring = json_object_new_string(buffer);
            json_object_array_add(jarray, jstring);
        }
        pclose(proc);

        if (json_object_array_length(jarray) > 0) {
            const char *json_str = json_object_to_json_string_ext(jarray, JSON_C_TO_STRING_PRETTY);
            printf("%s\n", json_str);

            char output_file[] = "log_search_results.json";
            FILE *output = fopen(output_file, "w");
            if (output) {
                fputs(json_str, output);
                fclose(output);
                printf("Exported to %s\n", output_file);
            } else {
                perror("Failed to open file for writing");
            }
        } else {
            printf("No results found.\n");
        }
        json_object_put(jarray);
    } else {
        perror("Failed to run command");
    }
    free(egrep_args);
}

void display_help(void) {
    printf(ANSI_COLOR_CYAN "Help Information\n" ANSI_COLOR_RESET);
    printf("Available commands:\n");
    printf("  (A)uth Log\n");
    printf("  (E)rror Log\n");
    printf("  (N)etwork Log\n");
    printf("  (R)un Regex\n");
    printf("  (I)P Search\n");
    printf("  (S)et Log Paths\n");
    printf("  (J)SON Export\n");
    printf("  (Q)uit\n");
}

void main_menu(void) {
    char *input;
    do {
        printf("\nMain Menu\n");
        printf("Choose an option: \n");
        printf("A: Auth Log\n");
        printf("E: Error Log\n");
        printf("N: Network Log\n");
        printf("R: Run Regex\n");
        printf("I: IP Search\n");
        printf("S: Set Log Paths\n");
        printf("J: JSON Export\n");
        printf("H: Help\n");
        printf("Q: Quit\n");
        input = get_user_input("> ");

        if (!input || strlen(input) != 1) {
            printf("Invalid input. Please enter one of the listed options.\n");
            continue;
        }

        switch (input[0]) {
            case 'A':
            case 'a':
                live_auth_log();
                break;
            case 'E':
            case 'e':
                live_error_log();
                break;
            case 'N':
            case 'n':
                live_network_log();
                break;
            case 'R':
            case 'r':
                run_regex();
                break;
            case 'I':
            case 'i':
                search_ip();
                break;
            case 'S':
            case 's':
                edit_log_paths();
                break;
            case 'J':
            case 'j':
                export_search_results_to_json();
                break;
            case 'H':
            case 'h':
                display_help();
                break;
            case 'Q':
            case 'q':
                free(input);
                return;
            default:
                printf("Invalid option. Please try again.\n");
        }
        free(input);
    } while (1);
}

int main(void) {
    signal(SIGINT, sigint_handler);
    main_menu();
    return 0;
}

void sigint_handler(int sig) {
    printf("\nInterrupt received. Returning to main menu...\n");
}
