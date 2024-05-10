#ifndef LOGS_HPP
#define LOGS_HPP

void live_auth_log(const char *log_search_path);
void live_error_log(const char *log_search_path);
void live_log(const char *log_search_path);
void live_network_log(const char *log_search_path);
void run_regex(const char *log_search_path);
void search_ip(const char *log_search_path);
void edit_log_paths(char *log_search_path);

#endif  // LOGS_HPP
