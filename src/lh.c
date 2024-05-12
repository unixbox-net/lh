#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

void execute_command_with_less(const char *cmd);
void find_and_process_logs(const char *log_search_path, const char *grep_args);

// Utility to execute a command and print its output using the less pager
void execute_command_with_less(const char *cmd) {
    char *full_command;
    size_t needed = snprintf(NULL, 0, "%s | less", cmd) + 1;
    full_command = malloc(needed);
    if (full_command) {
        snprintf(full_command, needed, "%s | less", cmd);
        system(full_command);
        free(full_command);
    } else {
        fprintf(stderr, "Failed to allocate memory for full command.\n");
    }
}

void find_and_process_logs(const char *log_search_path, const char *grep_args) {
    char command[8192];
    snprintf(command, sizeof(command), "grep -P '%s' $(find %s -type f -name '*.log')", grep_args, log_search_path);
    execute_command_with_less(command);
}

void live_auth_log(const char *log_search_path) {
    find_and_process_logs(log_search_path, "authentication|permission denied");
}

void live_error_log(const char *log_search_path) {
    find_and_process_logs(log_search_path, "error|failure|critical");
}

void live_network_log(const char *log_search_path) {
    find_and_process_logs(log_search_path, "http|ftp|ssh");
}

void live_log(const char *log_search_path) {
    char command[8192];
    snprintf(command, sizeof(command), "cat $(find %s -type f -name '*.log')", log_search_path);
    execute_command_with_less(command);
}

void main_menu() {
    char *option;
    printf(ANSI_COLOR_GREEN "Main Menu: [A]uth [E]rror [L]og [N]etwork [R]egEx [I]P [S]et [J]SON [H]elp [Q]uit\n" ANSI_COLOR_RESET);
    while (1) {
        option = readline("> ");
        if (option == NULL) continue;

        switch (option[0]) {
            case 'A': case 'a': live_auth_log(log_search_path); break;
            case 'E': case 'e': live_error_log(log_search_path); break;
            case 'L': case 'l': live_log(log_search_path); break;
            case 'N': case 'n': live_network_log(log_search_path); break;
            case 'Q': case 'q': free(option); return;
            default: printf(ANSI_COLOR_RED "Invalid option.\n" ANSI_COLOR_RESET);
        }
        free(option);
    }
}

int main() {
    main_menu();
    return 0;
}
