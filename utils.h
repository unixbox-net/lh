#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

#define BUFFER_SIZE 4096

// Function prototypes
void find_logs_command(char *buffer, size_t size, const char *search_path);
void display_buffer_with_less(const char *buffer, size_t length);
void run_command_with_buffer(const char *cmd, void (*buffer_action)(const char *, size_t));
char *get_user_input(const char *prompt);
int sanitize_input(char *input);
void sigint_handler(int sig);
void save_log_paths(const char *log_search_path);
void load_log_paths(char *log_search_path, size_t buffer_size);

#endif // UTILS_H
