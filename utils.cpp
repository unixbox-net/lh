#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <readline/readline.h>
#include <readline/history.h>
#include "utils.hpp"

#define CONFIG_FILE "/etc/loghog.conf"

std::string get_user_input(const std::string &prompt) {
    char *input = readline(prompt.c_str());
    if (input && *input) add_history(input);
    std::string result = input ? std::string(input) : "";
    free(input);
    return result;
}

bool sanitize_input(std::string &input) {
    static const std::string invalid_chars = ";&|><";
    return input.find_first_of(invalid_chars) == std::string::npos;
}

void find_logs_command(char *buffer, size_t size, const char *log_paths) {
    snprintf(buffer, size, "find %s -type f -readable 2>/dev/null -exec cat {} +", log_paths);
}

void run_command_with_buffer(const char *cmd, FILE *out) {
    FILE *proc = popen(cmd, "r");
    if (!proc) {
        std::cerr << "Error executing command.\n";
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), proc)) {
        if (out) {
            fputs(buffer, out);
        } else {
            std::cout << buffer;
        }
    }
    pclose(proc);
}

void save_log_paths(const char *log_paths) {
    std::ofstream config_file(CONFIG_FILE);
    if (config_file.is_open()) {
        config_file << log_paths;
        config_file.close();
    } else {
        std::cerr << "Unable to write to config file.\n";
    }
}

void load_log_paths(char *log_paths, size_t buffer_size) {
    std::ifstream config_file(CONFIG_FILE);
    if (config_file.is_open()) {
        std::stringstream ss;
        ss << config_file.rdbuf();
        std::string all_paths = ss.str();
        strncpy(log_paths, all_paths.c_str(), buffer_size - 1);
        log_paths[buffer_size - 1] = '\0';
        config_file.close();
    } else {
        std::cerr << "Error: No log paths found in config file.\n";
    }
}
