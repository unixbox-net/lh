#include <iostream>
#include <fstream>
#include <sstream>
#include "json_export.hpp"
#include "utils.hpp"

extern char log_search_path[BUFFER_SIZE];

void export_search_results_to_json(const char* log_search_path) {
    std::string egrep_args = get_user_input("\nRegEX / Text > ");
    if (!sanitize_input(egrep_args)) return;

    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE * 2];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=never -i \"%s\"", find_cmd, egrep_args.c_str());

    FILE* proc = popen(cmd, "r");
    if (!proc) {
        perror("popen");
        return;
    }

    std::ofstream ofs("log_search_results.json");
    ofs << "[\n";

    bool first = true;
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), proc)) {
        if (!first) {
            ofs << ",\n";
        }
        first = false;

        std::string line = buffer;
        std::string sanitized = sanitize_input(line);
        ofs << "  { \"log\": \"" << sanitized << "\" }";
    }

    ofs << "\n]\n";
    pclose(proc);

    std::cout << "\nExported search results to log_search_results.json\n";
}
