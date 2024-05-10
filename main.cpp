#include <csignal>
#include <iostream>
#include "logs.hpp"
#include "json_export.hpp"
#include "utils.hpp"

char log_search_path[BUFFER_SIZE] = "/var/lib/docker /var/log";

void sigint_handler(int sig) {
    std::cout << "\nCaught signal " << sig << ", returning to menu...\n";
    std::cout.flush();
}

void main_menu() {
    std::string option;
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
        std::getline(std::cin, option);

        if (option == "1") {
            live_auth_log(log_search_path);
        } else if (option == "2") {
            live_error_log(log_search_path);
        } else if (option == "3") {
            live_log(log_search_path);
        } else if (option == "4") {
            live_network_log(log_search_path);
        } else if (option == "5") {
            run_regex(log_search_path);
        } else if (option == "6") {
            search_ip(log_search_path);
        } else if (option == "7") {
            edit_log_paths(log_search_path);
        } else if (option == "8") {
            export_search_results_to_json(log_search_path);
        } else if (option == "9") {
            exit(0);
        } else {
            std::cout << "Invalid option. Try again.\n";
        }
    }
}

int main() {
    load_log_paths(log_search_path, BUFFER_SIZE);
    std::signal(SIGINT, sigint_handler);
    main_menu();
    return 0;
}
