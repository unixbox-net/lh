#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
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

void handle_interrupt(int sig);
void execute_tail_with_less(const char *cmd);

// Signal handling setup
void setup_signal_handling() {
    struct sigaction sa;
    sa.sa_handler = handle_interrupt;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

// Executes tail on log files and pipes to less upon interruption
void execute_tail_with_less(const char *cmd) {
    printf("Monitoring logs... Press CTRL+C to stop and review logs.\n");

    FILE *pipe = popen(cmd, "r");
    if (pipe == NULL) {
        perror("popen failed");
        return;
    }

    char buffer[1024];
    size_t bytes_read;
    char *full_output = NULL;
    size_t total_size = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        full_output = realloc(full_output, total_size + bytes_read + 1);
        if (!full_output) {
            perror("realloc failed");
            free(full_output);
            pclose(pipe);
            return;
        }
        memcpy(full_output + total_size, buffer, bytes_read);
        total_size += bytes_read;
        full_output[total_size] = '\0';
    }

    pclose(pipe);

    // Handle output with less
    if (full_output) {
        printf("Reviewing buffered logs...\n");
        FILE *tmp_file = fopen("/tmp/loghog_output.txt", "w+");
        if (tmp_file) {
            fwrite(full_output, 1, total_size, tmp_file);
            fclose(tmp_file);
            char less_cmd[256];
            snprintf(less_cmd, sizeof(less_cmd), "less /tmp/loghog_output.txt");
            system(less_cmd);
            remove("/tmp/loghog_output.txt");
        }
        free(full_output);
    }
}

void handle_interrupt(int sig) {
    printf("\nInterrupt received. Exiting tail...\n");
    // Exit the current loop or condition that keeps tail running
    exit(0);
}

void live_log(const char *log_search_path) {
    char command[8192];
    snprintf(command, sizeof(command), "tail -f $(find %s -type f -name '*.log' | sort)", log_search_path);
    execute_tail_with_less(command);
}

int main() {
    setup_signal_handling();
    live_log(log_search_path);
    return 0;
}
