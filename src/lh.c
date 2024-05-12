#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <json-c/json.h>
#include <readline/readline.h>
#include <readline/history.h>

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

char log_search_path[4096] = "/var/log"; // Default log path

void display_with_less(const char *content);
char* build_command(const char *base_cmd, const char *pattern);
void execute_command(const char *cmd, void (*action)(const char *, size_t));
void action_print(const char *data, size_t length);
void sigint_handler(int sig);

// Main menu functions
void live_auth_log(const char *log_search_path);
void live_error_log(const char *log_search_path);
void live_log(const char *log_search_path);
void live_network_log(const char *log_search_path);
void run_regex(const char *log_search_path);
void search_ip(const char *log_search_path);
void edit_log_paths();
void export_search_results_to_json(const char *log_search_path);
void display_help();

int main() {
    signal(SIGINT, sigint_handler);
    main_menu();
    return 0;
}

void display_with_less(const char *content) {
    char tmp_filename[] = "/tmp/loghogXXXXXX";
    int tmp_fd = mkstemp(tmp_filename);
    if (tmp_fd == -1) {
        perror("mkstemp failed");
        return;
    }

    FILE *tmp_file = fdopen(tmp_fd, "w");
    if (tmp_file == NULL) {
        perror("fdopen failed");
        close(tmp_fd);
        return;
    }

    fprintf(tmp_file, "%s", content);
    fflush(tmp_file);
    fclose(tmp_file);

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "less %s", tmp_filename);
    system(cmd); // Consider replacing system with a more secure alternative

    remove(tmp_filename);
}

char* build_command(const char *base_cmd, const char *pattern) {
    size_t needed = snprintf(NULL, 0, pattern, base_cmd) + 1;
    char *cmd = malloc(needed);
    if (cmd) {
        snprintf(cmd, needed, pattern, base_cmd);
    } else {
        fprintf(stderr, "Failed to allocate memory for command.\n");
    }
    return cmd;
}

void execute_command(const char *cmd, void (*action)(const char *, size_t)) {
    FILE *proc = popen(cmd, "r");
    if (proc == NULL) {
        perror("popen failed");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, proc)) != -1) {
        action(line, read);
    }
    free(line);
    pclose(proc);
}

void action_print(const char *data, size_t length) {
    printf("%s", data);
}

void sigint_handler(int sig) {
    printf("\nInterrupted. Returning to main menu...\n");
}

void live_auth_log(const char *log_search_path) {
    char *cmd = build_command(log_search_path, "grep -P 'authentication|permission denied' %s");
    if (cmd) {
        execute_command(cmd, action_print);
        free(cmd);
    }
}

void live_error_log(const char *log_search_path) {
    char *cmd = build_command(log_search_path, "grep -P 'error|failure|critical' %s");
    if (cmd) {
        execute_command(cmd, action_print);
        free(cmd);
    }
}

void live_log(const char *log_search_path) {
    char *cmd = build_command(log_search_path, "cat %s");
    if (cmd) {
        execute_command(cmd, action_print);
        free(cmd);
    }
}

void live_network_log(const char *log_search_path) {
    char *cmd = build_command(log_search_path, "grep -P 'http|ftp|ssh' %s");
    if (cmd) {
        execute_command(cmd, action_print);
        free(cmd);
    }
}

void run_regex(const char *log_search_path) {
    char *regex = readline("Enter regex: ");
    if (regex) {
        char *cmd = build_command(log_search_path, regex);
        if (cmd) {
            execute_command(cmd, action_print);
            free(cmd);
        }
        free(regex);
    }
}

void search_ip(const char *log_search_path) {
    char *ip_regex = readline("Enter IP or regex: ");
    if (ip_regex) {
        char *cmd = build_command(log_search_path, ip_regex);
        if (cmd) {
            execute_command(cmd, action_print);
            free(cmd);
        }
        free(ip_regex);
    }
}

void edit_log_paths() {
    char *new_paths = readline("Enter new log paths: ");
    if (new_paths && strlen(new_paths) < sizeof(log_search_path)) {
        strcpy(log_search_path, new_paths);
        printf("Log paths updated to: %s\n", log_search_path);
    } else {
        printf("Input too long or invalid.\n");
    }
    free(new_paths);
}

void export_search_results_to_json(const char *log_search_path) {
    // This function will implement JSON export
}

void display_help() {
    // This function will display help information
}

void main_menu() {
    char *option;
    while (1) {
        printf("Main Menu: [A]uth [E]rror [L]og [N]etwork [R]egEx [I]P [S]et [J]SON [H]elp [Q]uit\n> ");
        option = readline(NULL);
        if (option == NULL) {
            continue;
        }

        switch (option[0]) {
            case 'A': live_auth_log(log_search_path); break;
            case 'E': live_error_log(log_search_path); break;
            case 'L': live_log(log_search_path); break;
            case 'N': live_network_log(log_search_path); break;
            case 'R': run_regex(log_search_path); break;
            case 'I': search_ip(log_search_path); break;
            case 'S': edit_log_paths(); break;
            case 'J': export_search_results_to_json(log_search_path); break;
            case 'H': display_help(); break;
            case 'Q': free(option); return;
            default: printf("Invalid option.\n");
        }
        free(option);
    }
}
