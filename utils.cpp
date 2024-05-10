#include <iostream>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <wait.h>
#include <cctype>
#include "utils.hpp"

volatile sig_atomic_t menu_flag = 1;

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
