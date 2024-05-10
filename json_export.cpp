// json_export.cpp
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include "json_export.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 4096

void export_search_results_to_json(const char* log_search_path) {
    std::string egrep_args = get_user_input("\nRegEX / Text > ");
    if (!sanitize_input(egrep_args)) return;

    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE * 2];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=never -i \"%s\"", find_cmd, egrep_args.c_str());

    FILE* proc = popen(cmd, "r");
    if (!proc) {
        std::cerr << "Error: Failed to run command.\n";
        return;
    }

    std::ofstream json_file("log_search_results.json");
    json_file << "{\"results\": [\n";

    char buffer[BUFFER_SIZE];
    bool first = true;
    while (fgets(buffer, sizeof(buffer), proc)) {
        std::string sanitized = sanitize_input(buffer);
        if (!first) {
            json_file << ",\n";
        }
        first = false;
        json_file << "\"" << sanitized << "\"";
    }
    json_file << "\n]}\n";

    pclose(proc);
    std::cout << "Exported search results to log_search_results.json\n";
}
