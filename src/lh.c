#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <json-c/json.h>
#include <readline/readline.h>
#include <readline/history.h>

#define BUFFER_SIZE 4096  // Buffer size for command strings and general usage

// ANSI color codes
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31;1m"
#define ANSI_COLOR_GREEN "\x1b[32;1m"
#define ANSI_COLOR_YELLOW "\x1b[33;1m"
#define ANSI_COLOR_BLUE "\x1b[34;1m"
#define ANSI_COLOR_MAGENTA "\x1b[35;1m"
#define ANSI_COLOR_CYAN "\x1b[36;1m"
#define ANSI_COLOR_WHITE "\x1b[37;1m"
#define ANSI_COLOR_DARK "\x1b[30m"

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

// Global variables
char log_search_path[BUFFER_SIZE] = "/var/log";

// Function definitions
void find_logs_command(char *buffer, size_t size, const char *search_path) {
    snprintf(buffer, size, "find %s -type f \\( -name '*.log' -o -name 'messages' -o -name 'cron' -o -name 'maillog' -o -name 'secure' -o -name 'firewalld' \\) -exec tail -f -n +1 {} +", search_path);
}

void display_buffer_with_less(const char *buffer) {
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

    fwrite(buffer, 1, strlen(buffer), tmp_file);
    fflush(tmp_file);
    fclose(tmp_file);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "less -R %s", tmp_filename);
    system(cmd);
    remove(tmp_filename);
}

void run_command_with_buffer(const char *cmd, void (*buffer_action)(const char *)) {
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
    }

    if (buffer_action && output) {
        buffer_action(output);
    }

    free(output);
    pclose(proc);
}
void live_error_log(void) {
    char cmd[BUFFER_SIZE];
    find_logs_command(cmd, sizeof(cmd), log_search_path);
    strncat(cmd, " | egrep --color=always -i \"\\b(?:error|fail(?:ed|ure)?|warn(?:ing)?|critical|socket|denied|refused|retry|reset|timeout|dns|network)\"", sizeof(cmd) - strlen(cmd), log_search_path);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

void live_network_log(void) {
    char cmd[BUFFER_SIZE];
    find_logs_command(cmd, sizeof(cmd), log_search_path);
    strncat(cmd, " | egrep --color=always -i 'https?://|ftps?://|telnet://|ssh://|sftp://|ldap(s)?://|nfs://|tftp://|gopher://|imap(s)?://|pop3(s)?://|smtp(s)?://|rtsp://|rtmp://|mms://|xmpp://|ipp://|xrdp://'", sizeof(cmd) - strlen(cmd), log_search_path);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

void run_regex(void) {
    char *egrep_args = get_user_input("\nRegEX > ");
    if (!sanitize_input(egrep_args)) {
        free(egrep_args);
        return;
    }
    char cmd[BUFFER_SIZE];
    find_logs_command(cmd, sizeof(cmd), log_search_path);
    strncat(cmd, " | egrep --color=always -i ", sizeof(cmd) - strlen(cmd));
    strncat(cmd, egrep_args, sizeof(cmd) - strlen(cmd));
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
    find_logs_command(cmd, sizeof(cmd), log_search_path);
    strncat(cmd, " | egrep --color=always -i ", sizeof(cmd) - strlen(cmd));
    strncat(cmd, ip_regex, sizeof(cmd) - strlen(cmd));
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
    printf(ANSI_COLOR_GREEN "Updated log paths: %s\n" ANSI_COLOR_RESET, log_search_path);
    free(new_paths);
}

void export_search_results_to_json(void) {
    char *egrep_args = get_user_input("\nRegEX / Text > ");
    if (!sanitize_input(egrep_args)) {
        free(egrep_args);
        return;
    }
    char cmd[BUFFER_SIZE];
    find_logs_command(cmd, sizeof(cmd), log_search_path);
    strncat(cmd, " | egrep --color=never -i ", sizeof(cmd) - strlen(cmd));
    strncat(cmd, egrep_args, sizeof(cmd) - strlen(cmd));
    char *output = NULL;
    FILE *proc = popen(cmd, "r");
    if (proc) {
        char buffer[BUFFER_SIZE];
        json_object *jarray = json_object_new_array();
        while (fgets(buffer, sizeof(buffer), proc) != NULL) {
            json_object *jentry = json_object_new_object();
            json_object_object_add(jentry, "log_entry", json_object_new_string(buffer));
            json_object_array_add(jarray, jentry);
        }
        pclose(proc);
        if (json_object_array_length(jarray) > 0) {
            FILE *outfile = fopen("log_search_results.json", "w");
            if (outfile) {
                fputs(json_object_to_json_string_ext(jarray, JSON_C_TO_STRING_PRETTY), outfile);
                fclose(outfile);
                printf("Logs have been exported to 'log_search_results.json'.\n");
            } else {
                perror("Failed to open file for writing");
            }
        } else {
            printf("No logs matched the criteria.\n");
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

        input = readline("> ");
        if (input && *input) {
            switch (input[0]) {
                case 'A': case 'a':
                    live_auth_log();
                    break;
                case 'E': case 'e':
                    live_error_log();
                    break;
                case 'N': case 'n':
                    live_network_log();
                    break;
                case 'R': case 'r':
                    run_regex();
                    break;
                case 'I': case 'i':
                    search_ip();
                    break;
                case 'S': case 's':
                    edit_log_paths();
                    break;
                case 'J': case 'j':
                    export_search_results_to_json();
                    break;
                case 'H': case 'h':
                    display_help();
                    break;
                case 'Q': case 'q':
                    free(input);
                    return;
                default:
                    printf("Invalid option.\n");
            }
        }
        free(input);
    } while (1);
}

void sigint_handler(int sig) {
    printf("Caught SIGINT, ignoring and returning to menu...\n");
}

int main(int argc, char **argv) {
    signal(SIGINT, sigint_handler);
    main_menu();
    return 0;
}
