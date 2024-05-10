#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

const size_t BUFFER_SIZE = 1024;

void sigint_handler(int sig);
void reset_menu_flag();
void display_buffer_with_less(const char* buffer, size_t size);
std::string sanitize_input(const std::string& input);
void find_logs_command(char* command, size_t size, const char* log_search_path);
std::string get_user_input(const std::string& prompt);
void run_command_with_buffer(const char* command, const char* filter);
void save_log_paths(const char* log_search_path);
void load_log_paths(char* log_search_path, size_t size);

#endif // UTILS_HPP
