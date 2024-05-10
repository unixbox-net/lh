// main.cpp
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include "logs.hpp"
#include "json_export.hpp"
#include "utils.hpp"

#define CONFIG_FILE "/etc/loghog.conf"
#define BUFFER_SIZE 4096

char log_search_path[BUFFER_SIZE] = "/var/log/messages /var/log/secure /var/log/audit/audit.log";

void load_log_paths(char* log_paths, size_t buffer_size) {
    std::ifstream config_file(CONFIG_FILE);
    if (!config_file) {
        std::cerr << "Error: No config file found at " << CONFIG_FILE << ".\n";
        return;
    }

    std::string line;
    std::vector<std::string> paths;
    while (std::getline(config_file, line)) {
        if (!line.empty() && line[0] != '#') {
            paths.push_back(line);
        }
    }

    std::string all_paths = "";
    for (const auto& path : paths) {
        all_paths += path + " ";
    }

    if (all_paths.empty()) {
        std::cerr << "Error: No log paths found in config file.\n";
    } else {
        strncpy(log_paths, all_paths.c_str(), buffer_size - 1);
    }
}

void main_menu() {
    while (true) {
        std::cout << "\n1) Live Auth Log"
                  << "\n2) Live Error Log"
                  << "\n3) Live Log"
                  << "\n4) Live Network Log"
                  << "\n5) Run Regex"
                  << "\n6) Search IP"
                  << "\n7) Edit Log Paths"
                  << "\n8) Export to JSON"
                  << "\n9) Quit\n> ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1: live_auth_log(log_search_path); break;
            case 2: live_error_log(log_search_path); break;
            case 3: live_log(log_search_path); break;
            case 4: live_network_log(log_search_path); break;
            case 5: run_regex(log_search_path); break;
            case 6: search_ip(log_search_path); break;
            case 7: edit_log_paths(log_search_path); break;
            case 8: export_search_results_to_json(log_search_path); break;
            case 9: return;
            default: std::cout << "Invalid choice. Please try again.\n"; break;
        }
    }
}

int main() {
    load_log_paths(log_search_path, BUFFER_SIZE);
    main_menu();
    return 0;
}
