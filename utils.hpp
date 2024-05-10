// utils.hpp
#ifndef UTILS_HPP
#define UTILS_HPP

std::string get_user_input(const std::string& prompt);
std::string sanitize_input(const std::string& input);
void run_command_with_buffer(const char* cmd, char* buffer);
void display_buffer_with_less(const char* file, size_t buffer_size);
void find_logs_command(char* find_cmd, size_t buffer_size, const char* log_search_path);
void save_log_paths(const char* log_paths);
void sigint_handler(int sig);
void reset_menu_flag();

#endif // UTILS_HPP
