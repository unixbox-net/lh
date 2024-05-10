#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

void find_logs_command(char* find_cmd, size_t buffer_size, const char* log_search_path);
void run_command_with_buffer(const char* command, char* buffer);
std::string get_user_input(const std::string& prompt);
bool sanitize_input(std::string& input);
void save_log_paths(const char* log_paths);
std::string load_log_paths_from_config();

#endif // UTILS_HPP
