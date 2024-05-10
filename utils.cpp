#include <iostream>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>
#include "utils.hpp"

#define CONFIG_FILE "/etc/loghog.conf"
#define BUFFER_SIZE 1024

void find_logs_command(char* find_cmd, size_t buffer_size, const char* log_search_path) {
    snprintf(find_cmd, buffer_size, "find %s -type f -print0 | xargs -0 grep --color=never -Hni --binary-files=text", log_search_path);
}

void run_command_with_buffer(const char* command, char* buffer) {
    FILE* proc = popen(command, "r");
    if (!proc) {
        std::cerr << "Error: Unable to execute command.\n";
        return;
    }
    if (buffer) {
        while (fgets(buffer, BUFFER_SIZE, proc)) {
            std::cout << buffer;
        }
    } else {
        char proc_buffer[BUFFER_SIZE];
        while (fgets(proc_buffer, sizeof(proc_buffer), proc)) {
            std::cout << proc_buffer;
        }
    }
    pclose(proc);
}

std::string get_user_input(const std::string& prompt) {
    char* input = readline(prompt.c_str());
    if (input && *input) {
        add_history(input);
    }
    std::string result(input ? input : "");
    free(input);
    return result;
}

bool sanitize_input(std::string& input) {
    if (input.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.|/*?\\ ") != std::string::npos) {
        std::cerr << "Invalid characters in input. Please try again.\n";
        return false;
    }
    return true;
}

void save_log_paths(const char* log_paths) {
    std::ofstream config_file(CONFIG_FILE);
    if (!config_file.is_open()) {
        std::cerr << "Error: Unable to open config file for writing.\n";
        return;
    }
    config_file << log_paths << "\n";
    config_file.close();
}

std::string load_log_paths_from_config() {
    std::ifstream config_file(CONFIG_FILE);
    if (!config_file.is_open()) {
        std::cerr << "Error: Unable to open config file for reading.\n";
        return "";
    }
    std::string all_paths;
    std::getline(config_file, all_paths);
    config_file.close();
    return all_paths;
}
