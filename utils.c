#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "utils.h"

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
    size_t output_capacity = BUFFER_SIZE; // Initial allocation
    output = malloc(output_capacity);
    if (!output) {
        perror("malloc");
        pclose(proc);
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), proc)) {
        size_t buffer_length = strlen(buffer);

        // Ensure output buffer can hold the new data
        if (total_length + buffer_length + 1 > output_capacity) {
            output_capacity *= 2;
            char *temp = realloc(output, output_capacity);
            if (!temp) {
                perror("realloc");
                free(output);
                pclose(proc);
                return;
            }
            output = temp;
        }

        // Append the new data to the output buffer
        memcpy(output + total_length, buffer, buffer_length);
        total_length += buffer_length;
        output[total_length] = '\0';

        fputs(buffer, stdout);
        fflush(stdout);
    }

    if (buffer_action && output) {
        buffer_action(output, total_length);
    }

    free(output);
    pclose(proc);
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
 * Validates the user input to ensure it is within bounds and allows specific regex characters.
 * @param input Input string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int sanitize_input(char *input) {
    if (input == NULL || strlen(input) == 0) {
        return 0;
    }

    if (strlen(input) >= BUFFER_SIZE - 1) {
        printf(ANSI_COLOR_RED "Input too long. Please try again.\n" ANSI_COLOR_RESET);
        return 0;
    }

    // Allow alphanumeric, space, and common regex syntax
    for (char *p = input; *p; p++) {
        if (!isalnum((unsigned char)*p) && *p != ' ' && *p != '-' && *p != '_' &&
            *p != '|' && *p != '.' && *p != '*' && *p != '^' && *p != '$' &&
            *p != '\\' && *p != '(' && *p != ')' && *p != '[' && *p != ']' &&
            *p != '+' && *p != '?' && *p != '{' && *p != '}') {
            printf(ANSI_COLOR_RED "Invalid characters in input. Please try again.\n" ANSI_COLOR_RESET);
            return 0;
        }
    }
    return 1;
}

/**
 * Handles the SIGINT signal to return to the main menu.
 * @param sig Signal number.
 */
void sigint_handler(int sig) {
    printf("\nReturning to menu...\n");
    fflush(stdout);
}
