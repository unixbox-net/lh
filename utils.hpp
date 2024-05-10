#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

#define BUFFER_SIZE 1024

std::string get_user_input(const std::string &prompt);
bool sanitize_input(std::string &input);
void find_logs_command(char *buffer, size_t size, const char *log_paths);
void run_command_with_buffer(const char *cmd, FILE *out = nullptr);
void save_log_paths(const char *log_paths);
void load_log_paths(char *log_paths, size_t buffer_size);

#endif // UTILS_HPP
