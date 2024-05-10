// utils.cpp
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <csignal>
#include <string>
#include <algorithm>
#include "utils.hpp"

#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"

#define BUFFER_SIZE 4096

volatile sig_atomic_t menu_flag = 0;

void sigint_handler(int sig) {
    menu_flag = 1;
    std::cout << "\n";
}

void reset_menu_flag() {
    menu_flag = 0;
}

std::string get_user_input(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

std::string sanitize_input(const std::string& input) {
    if (input.find_first_of("|;&><$!{}[]*?") != std::string::npos) {
        std::cout << ANSI_COLOR_RED "Invalid characters in input. Please try again.\n" ANSI_COLOR_RESET;
        return "";
    }
    return input;
}

void run_command_with_buffer(const char* cmd, char* buffer) {
    signal(SIGINT, sigint_handler);
    menu_flag = 0;

    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        std::cerr << "Error: Unable to execute command.\n";
        return;
    }

    int tmp_fd = open("/tmp/loghog_buffer.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (tmp_fd < 0) {
        std::cerr << "Error: Unable to open buffer file.\n";
        pclose(pipe);
        return;
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), pipe) && !menu_flag) {
        std::cout << line;
        write(tmp_fd, line, strlen(line));
    }
    pclose(pipe);
    close(tmp_fd);

    if (menu_flag) {
        display_buffer_with_less("/tmp/loghog_buffer.txt", BUFFER_SIZE);
    }

    std::cout << "\n  \n";
    signal(SIGINT, SIG_DFL);
}

void display_buffer_with_less(const char* file, size_t buffer_size) {
    char cmd[buffer_size];
    snprintf(cmd, sizeof(cmd), "less %s", file);
    system(cmd);
}

void find_logs_command(char* find_cmd, size_t buffer_size, const char* log_search_path) {
    snprintf(find_cmd, buffer_size, "find %s -type f -exec cat {} +", log_search_path);
}

void save_log_paths(const char* log_paths) {
    std::ofstream config_file(CONFIG_FILE);
    if (config_file) {
        config_file << log_paths << std::endl;
    } else {
        std::cerr << "Error: Unable to open config file for writing.\n";
    }
}
