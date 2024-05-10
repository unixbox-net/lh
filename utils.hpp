#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

#define BUFFER_SIZE 4096
#define CONFIG_FILE "/etc/loghog.conf"

void find_logs_command(char *buffer, size_t size, const char *search_path);
void display_buffer_with_less(const char *buffer, size_t length);
void run_command_with_buffer(const char *cmd, void (*buffer_action)(const char *, size_t));
std::string get_user_input(const std::string &prompt);
bool sanitize_input(std::string &input);
void sigint_handler(int sig);
void save_log_paths(const char *log_search_path);
void load_log_paths(char *log_search_path, size_t buffer_size);

#endif  // UTILS_HPP
