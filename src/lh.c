#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31;1m"
#define ANSI_COLOR_GREEN "\x1b[32;1m"
#define ANSI_COLOR_BLUE "\x1b[34;1m"

volatile sig_atomic_t keepRunning = 1;

void sigint_handler(int sig) {
    keepRunning = 0;
}

char* find_logs_command() {
    return "find /var/lib/docker /var/log -type f \\( -name '*.log' -o -name 'messages' -o -name 'cron' -o -name 'maillog' -o -name 'secure' -o -name 'firewalld' \\) -exec tail -f -n +1 {} +";
}

void display_buffer_with_less(const char *filename) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "less %s", filename);
    system(cmd);
}

void run_command_with_buffer(const char *cmd) {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Child process
        close(fd[0]); // Close unused read end
        dup2(fd[1], STDOUT_FILENO); // Redirect stdout to pipe
        dup2(fd[1], STDERR_FILENO); // Redirect stderr to pipe
        system(cmd);
        close(fd[1]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        close(fd[1]); // Close unused write end
        char buffer[1024];
        FILE *tmp = tmpfile();
        if (!tmp) {
            perror("tmpfile");
            close(fd[0]);
            return;
        }

        while (keepRunning && read(fd[0], buffer, sizeof(buffer)) > 0) {
            fwrite(buffer, 1, strlen(buffer), tmp);
            fflush(tmp);
        }

        fseek(tmp, 0, SEEK_SET);
        char c;
        while ((c = fgetc(tmp)) != EOF) {
            putchar(c);
        }

        fclose(tmp);
        close(fd[0]);

        waitpid(pid, NULL, 0); // Wait for child process to finish
        if (!keepRunning) {
            display_buffer_with_less("/proc/self/fd/3"); // This might need adjustment based on how your system handles fd
        }
    }
}

int main() {
    signal(SIGINT, sigint_handler);
    char *cmd = find_logs_command();
    run_command_with_buffer(cmd);
    return 0;
}
