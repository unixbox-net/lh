#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <csignal>
#include <fstream>
#include <unistd.h>
#include "utils.hpp"

static std::string global_output_buffer;
static bool is_in_menu = false;

void find_logs_command(char *buffer, size_t size, const char *search_path) {
    snprintf(buffer, size, "find %s -type f \\( -name '*.log' -o -name 'messages' -o -name 'cron' -o -name 'maillog' -o -name 'secure' -o -name 'firewalld' \\) -exec tail -f -n +1 {} +", search_path);
}

void display_buffer_with_less(const char *buffer, size_t length) {
    char tmp_filename[] = "/tmp/logsearchXXXXXX";
    int tmp_fd = mkstemp(tmp_filename);
    if (tmp_fd == -1) {
        perror("mkstemp");
        return;
    }

    FILE *tmp_file = fdopen(tmp_fd, "w+");
    if (tmp_file == nullptr) {
        perror("fdopen");
        close(tmp_fd);
        return;
    }

    fwrite(buffer, 1, length, tmp_file);
    fflush(tmp_file);
    fclose(tmp_file);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "less -R %s", tmp_filename);
    system(cmd);
    remove(tmp_filename);
}

void run_command_with_buffer(const char *cmd, void (*buffer_action)(const char *, size_t)) {
    FILE *proc = popen(cmd, "r");
    if (proc == nullptr) {
        perror("popen");
        return;
    }

    global_output_buffer.clear(); // Reset the global output buffer
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, proc) != nullptr) {
        global_output_buffer += buffer;
        std::cout << buffer;  // Immediate output to the console
        std::cout.flush();
    }

    pclose(proc);

    if (buffer_action) {
        buffer_action(global_output_buffer.c_str(), global_output_buffer.size());
    }
}

void sigint_handler(int sig) {
    if (!is_in_menu) {
        std::cout << "\nCaught signal " << sig << ", returning to menu...\n";
        std::cout.flush();
        is_in_menu = true;

        if (!global_output_buffer.empty()) {
            display_buffer_with_less(global_output_buffer.c_str(), global_output_buffer.size());
        }
    } else {
        std::cout << "\nExiting application...\n";
        exit(0);
    }
}

std::string get_user_input(const std::string &prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

bool sanitize_input(std::string &input) {
    if (input.empty() || input.size() >= BUFFER_SIZE) {
        std::cerr << "Invalid input. Please try again.\n";
        return false;
    }

    for (char c : input) {
        if (!isalnum(c) && c != ' ' && c != '-' && c != '_' && c != '|' && c != '.' &&
            c != '*' && c != '^' && c != '$' && c != '\\' && c != '(' && c != ')' &&
            c != '[' && c != ']' && c != '+' && c != '?' && c != '{' && c != '}' && c != '/') {
            std::cerr << "Invalid characters in input. Please try again.\n";
            return false;
        }
    }

    return true;
}

void save_log_paths(const char *log_search_path) {
    std::ofstream config_file(CONFIG_FILE);
    if (!config_file) {
        perror("ofstream");
        return;
    }
    config_file << log_search_path << std::endl;
}

void load_log_paths(char *log_search_path, size_t buffer_size) {
    std::ifstream config_file(CONFIG_FILE);
    if (!config_file) {
        strncpy(log_search_path, "/var/lib/docker /var/log", buffer_size - 1);
        log_search_path[BUFFER_SIZE - 1] = '\0';
        return;
    }

    std::string line;
    if (std::getline(config_file, line)) {
        strncpy(log_search_path, line.c_str(), buffer_size - 1);
        log_search_path[BUFFER_SIZE - 1] = '\0';
    } else {
        strncpy(log_search_path, "/var/lib/docker /var/log", buffer_size - 1);
        log_search_path[BUFFER_SIZE - 1] = '\0';
    }
}
