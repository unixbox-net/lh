#ifndef LOGS_HPP
#define LOGS_HPP

#include <string>

#define BUFFER_SIZE 1024

void live_auth_log(const char *unused);
void live_error_log(const char *unused);
void live_log(const char *unused);
void live_network_log(const char *unused);
void run_regex(const char *unused);
void search_ip(const char *unused);
void edit_log_paths(char *log_search_path);

#endif // LOGS_HPP
