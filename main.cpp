#include <iostream>
#include "logs.hpp"
#include "json_export.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 1024
char log_search_path[BUFFER_SIZE] = "/var/log/messages /var/log/secure /var/log/audit/audit.log";

void main_menu() {
    int choice = -1;
    while (choice != 9) {
        std::cout << "\n1) Live Auth Log\n"
                  << "2) Live Error Log\n"
                  << "3) Live Log\n"
                  << "4) Live Network Log\n"
                  << "5) Run Regex\n"
                  << "6) Search IP\n"
                  << "7) Edit Log Paths\n"
                  << "8) Export to JSON\n"
                  << "9) Quit\n> ";
        std::cin >> choice;
        std::cin.ignore(); // Ignore newline character

        switch (choice) {
            case 1:
                live_auth_log(log_search_path);
                break;
            case 2:
                live_error_log(log_search_path);
                break;
            case 3:
                live_log(log_search_path);
                break;
            case 4:
                live_network_log(log_search_path);
                break;
            case 5:
                run_regex(log_search_path);
                break;
            case 6:
                search_ip(log_search_path);
                break;
            case 7:
                edit_log_paths(log_search_path);
                break;
            case 8:
                export_search_results_to_json(log_search_path);
                break;
            case 9:
                std::cout << "\nExiting...\n";
                break;
            default:
                std::cout << "\nInvalid choice!\n";
                break;
        }
    }
}

int main() {
    load_log_paths(log_search_path, BUFFER_SIZE);
    main_menu();
    return 0;
}
