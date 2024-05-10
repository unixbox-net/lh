#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "logs.h"

#define EGRP_AUTH "authentication(\\s*failed)?|permission(\\s*denied)?|invalid\\s*(user|password|token)|(unauthorized|illegal)\\s*(access|attempt)|SQL\\s*injection|cross-site\\s*(scripting|request\\s*Forgery)|directory\\s*traversal|(brute-?force|DoS|DDoS)\\s*attack|(vulnerability|exploit)\\s*(detected|scan)"
#define EGRP_ERROR "\\b(?:error|fail(?:ed|ure)?|warn(?:ing)?|critical|socket|denied|refused|retry|reset|timeout|dns|network)"
#define EGRP_NETPROTOCOL "https?://|ftps?://|telnet://|ssh://|sftp://|ldap(s)?://|nfs://|tftp://|gopher://|imap(s)?://|pop3(s)?://|smtp(s)?://|rtsp://|rtmp://|mms://|xmpp://|ipp://|xrdp://"

/**
 * Searches logs for authentication-related issues and displays them using 'less'.
 * @param log_search_path Paths to search for logs.
 */
void live_auth_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"%s\"", find_cmd, EGRP_AUTH);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

/**
 * Searches logs for error-related issues and displays them using 'less'.
 * @param log_search_path Paths to search for logs.
 */
void live_error_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"%s\"", find_cmd, EGRP_ERROR);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

/**
 * Displays all logs using 'less'.
 * @param log_search_path Paths to search for logs.
 */
void live_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s", find_cmd);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

/**
 * Searches logs for network protocol-related issues and displays them using 'less'.
 * @param log_search_path Paths to search for logs.
 */
void live_network_log(const char *log_search_path) {
    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"%s\"", find_cmd, EGRP_NETPROTOCOL);
    run_command_with_buffer(cmd, display_buffer_with_less);
}

/**
 * Prompts the user to enter a regular expression and searches the logs.
 * @param log_search_path Paths to search for logs.
 */
void run_regex(const char *log_search_path) {
    char *egrep_args = get_user_input("\nRegEX > ");
    if (!sanitize_input(egrep_args)) {
        free(egrep_args);
        return;
    }

    char cmd[BUFFER_SIZE * 2]; // Ensure sufficient buffer size
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    if (snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"%s\"", find_cmd, egrep_args) >= sizeof(cmd)) {
        printf(ANSI_COLOR_RED "Command is too long. Please refine your search.\n" ANSI_COLOR_RESET);
        free(egrep_args);
        return;
    }

    run_command_with_buffer(cmd, display_buffer_with_less);
    free(egrep_args);
}

/**
 * Prompts the user to enter an IP or regex pattern and searches the logs.
 * @param log_search_path Paths to search for logs.
 */
void search_ip(const char *log_search_path) {
    char *ip_regex = get_user_input("\nIP / RegEX > ");
    if (!sanitize_input(ip_regex)) {
        free(ip_regex);
        return;
    }

    char cmd[BUFFER_SIZE * 2]; // Ensure sufficient buffer size
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    if (snprintf(cmd, sizeof(cmd), "%s | egrep --color=always -i \"%s\"", find_cmd, ip_regex) >= sizeof(cmd)) {
        printf(ANSI_COLOR_RED "Command is too long. Please refine your search.\n" ANSI_COLOR_RESET);
        free(ip_regex);
        return;
    }

    run_command_with_buffer(cmd, display_buffer_with_less);
    free(ip_regex);
}

/**
 * Allows the user to edit the log search paths.
 * @param log_search_path Paths to search for logs.
 */
void edit_log_paths(char *log_search_path) {
    char *new_paths = get_user_input("\nCurrent log paths: /var/lib/docker /var/log\nEnter new log paths (separated by spaces) > ");
    if (!sanitize_input(new_paths)) {
        free(new_paths);
        return;
    }

    strncpy(log_search_path, new_paths, BUFFER_SIZE - 1);
    log_search_path[BUFFER_SIZE - 1] = '\0';
    free(new_paths);

    save_log_paths(log_search_path);
    printf(ANSI_COLOR_GREEN "Updated log paths: %s\n" ANSI_COLOR_RESET, log_search_path);
}
