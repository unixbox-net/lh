#include <iostream>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <wait.h>
#include <fstream>
#include <sstream>
#include "utils.hpp"

volatile sig_atomic_t menu_flag = 1;
const char* CONFIG_FILE = "/etc/loghog.conf";

void sigint_handler(int sig) {
    menu_flag = 0;
    std::cout << "  " << std::endl; // Two blank spaces
}

void reset_menu_flag() {
    menu_flag = 1;
}

void display_buffer_with_less(const char* buffer, size_t size) {
    char tmp_file_template[] = "/tmp/loghog.XXXXXX";
    int tmp_fd = mkstemp(tmp_file_template);

    if (tmp_fd == -1) {
        perror("mkstemp");
        return;
    }

    if (write(tmp_fd, buffer, size) != size) {
        perror("write");
        close(tmp_fd);
        return;
    }

    lseek(tmp_fd, 0, SEEK_SET);

    if (fork() == 0) {
        execlp("less", "less", "-R", tmp_file_template, nullptr);
        perror("execlp");
        _exit(EXIT_FAILURE);
    }

    close(tmp_fd);
    wait(nullptr);
}

std::string sanitize_input(const std::string& input) {
    std::string sanitized;
    for (char ch : input) {
        if (std::isalnum(ch) || std::isspace(ch) || ch == '/' || ch == '.' || ch == '-' || ch == '_' || ch == '*' || ch == '|' || ch == '(' || ch == ')') {
            sanitized += ch;
        }
    }

    if (sanitized != input) {
        std::cout << "  " << std::endl; // Two blank spaces
    }

    return sanitized;
}

void find_logs_command(char* command, size_t size, const char* log_search_path) {
    std::stringstream ss;
    ss << "find " << log_search_path << " -type f \\( -name \"*.log\" -o -name \"*.txt\" -o -name \"*.err\" \\)";
    std::string cmd = ss.str();
    strncpy(command, cmd.c_str(), size - 1);
    command[size - 1] = '\0';
}

std::string get_user_input(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return sanitize_input(input);
}

void run_command_with_buffer(const char* command, const char* filter) {
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        perror("popen");
        return;
    }

    std::stringstream buffer;
    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), pipe)) {
        buffer << line;
    }

    pclose(pipe);

    std::string output = buffer.str();
    if (filter && strlen(filter) > 0) {
        std::cout << "Filtered results:\n";
        std::cout << output << std::endl;
    } else {
        display_buffer_with_less(output.c_str(), output.size());
    }
}

void save_log_paths(const char* log_search_path) {
    std::ofstream ofs(CONFIG_FILE);
    if (!ofs) {
        std::cerr << "Error: Could not open config file for writing.\n";
        return;
    }
    ofs << "LOG_PATHS=" << log_search_path << '\n';
}

void load_log_paths(char* log_search_path, size_t size) {
    std::ifstream ifs(CONFIG_FILE);
    if (!ifs) {
        std::cerr << "Error: Could not open config file for reading.\n";
        return;
    }

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.find("LOG_PATHS=") == 0) {
            strncpy(log_search_path, line.substr(10).c_str(), size - 1);
            log_search_path[size - 1] = '\0';
            return;
        }
    }

    std::cerr << "Error: No log paths found in config file.\n";
}
