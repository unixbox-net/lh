#include "logs.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "utils.hpp"

extern char log_search_path[BUFFER_SIZE];

void live_auth_log(const char *unused) {
    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | grep --color=always -i \"authentication(\\s*failed)?|permission(\\s*denied)?|invalid\\s*(user|password|token)|(unauthorized|illegal)\\s*(access|attempt)\"", find_cmd);
    run_command_with_buffer(cmd, nullptr);
}

void live_error_log(const char *unused) {
    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | grep --color=always -i \"\\b(?:error|fail(?:ed|ure)?|warn(?:ing)?|critical|socket|denied|refused|retry|reset|timeout|dns|network)\"", find_cmd);
    run_command_with_buffer(cmd, nullptr);
}

void live_log(const char *unused) {
    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s", find_cmd);
    run_command_with_buffer(cmd, nullptr);
}

void live_network_log(const char *unused) {
    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | grep --color=always -i 'https?://|ftps?://|telnet://|ssh://|sftp://|ldap(s)?|smtp(s)?'", find_cmd);
    run_command_with_buffer(cmd, nullptr);
}

void run_regex(const char *unused) {
    std::string regex = get_user_input("\nRegEX > ");
    if (regex.empty() || !sanitize_input(regex)) return;

    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | grep --color=always -i \"%s\"", find_cmd, regex.c_str());
    run_command_with_buffer(cmd, nullptr);
}

void search_ip(const char *unused) {
    std::string ip_regex = get_user_input("\nIP / RegEX > ");
    if (ip_regex.empty() || !sanitize_input(ip_regex)) return;

    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | grep --color=always -i \"%s\"", find_cmd, ip_regex.c_str());
    run_command_with_buffer(cmd, nullptr);
}

void edit_log_paths(char *log_search_path) {
    std::string new_paths = get_user_input("\nCurrent log paths: " + std::string(log_search_path) + "\nEnter new log paths (separated by spaces) > ");
    if (new_paths.empty() || !sanitize_input(new_paths)) return;

    strncpy(log_search_path, new_paths.c_str(), BUFFER_SIZE - 1);
    log_search_path[BUFFER_SIZE - 1] = '\0';
    save_log_paths(log_search_path);
    std::cout << "\nUpdated log paths: " << log_search_path << "\n";
}
