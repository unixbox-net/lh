#include <iostream>
#include <cstring>
#include "logs.hpp"
#include "json_export.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 1024

char log_search_path[BUFFER_SIZE] = "/var/lib/docker /var/log";

void load_log_paths(char* log_paths, size_t buffer_size);

void main_menu() {
    char choice[3];
    while (true) {
        std::cout << "\n1) Live Auth Log\n"
                  << "2) Live Error Log\n"
                  << "3) Live Log\n"
                  << "4) Live Network Log\n"
                  << "5) Run Regex\n"
                  << "6) Search IP\n"
                  << "7) Edit Log Paths\n"
                  << "8) Export to JSON\n"
                  << "9) Quit\n"
                  << "> ";
        std::cin >> choice;
        switch (choice[0]) {
            case '1':
                live_auth_log(log_search_path);
                break;
            case '2':
                live_error_log(log_search_path);
                break;
            case '3':
                live_log(log_search_path);
                break;
            case '4':
                live_network_log(log_search_path);
                break;
            case '5':
                run_regex(log_search_path);
                break;
            case '6':
                search_ip(log_search_path);
                break;
            case '7':
                edit_log_paths(log_search_path);
                break;
            case '8':
                export_search_results_to_json(log_search_path);
                break;
            case '9':
                return;
            default:
                std::cout << "Invalid choice!\n";
        }
    }
}

int main() {
    load_log_paths(log_search_path, BUFFER_SIZE);
    main_menu();
    return 0;
}

void load_log_paths(char* log_paths, size_t buffer_size) {
    std::string all_paths = load_log_paths_from_config();
    if (all_paths.empty()) {
        std::cerr << "Error: No log paths found in config file.\n";
    } else {
        strncpy(log_paths, all_paths.c_str(), buffer_size - 1);
        log_paths[buffer_size - 1] = '\0';
    }
}
