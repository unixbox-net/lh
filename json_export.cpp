#include <iostream>
#include <cstdio>
#include <cstring>
#include <json-c/json.h>
#include "json_export.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 1024

void export_search_results_to_json(const char* log_search_path) {
    std::string egrep_args = get_user_input("\nRegEX / Text > ");
    if (!sanitize_input(egrep_args)) return;
    char find_cmd[BUFFER_SIZE * 2];
    char cmd[BUFFER_SIZE * 2];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=never -i \"%s\"", find_cmd, egrep_args.c_str());
    FILE* proc = popen(cmd, "r");

    if (!proc) {
        std::cerr << "Error: Unable to execute command.\n";
        return;
    }

    json_object* json_array = json_object_new_array();
    char buffer[BUFFER_SIZE];

    while (fgets(buffer, sizeof(buffer), proc)) {
        std::string line(buffer);
        line.erase(line.find_last_not_of("\n\r") + 1); // Strip newline
        if (!sanitize_input(line)) continue;
        json_object* json_entry = json_object_new_object();
        json_object_object_add(json_entry, "log", json_object_new_string(line.c_str()));
        json_object_array_add(json_array, json_entry);
    }

    pclose(proc);

    const char* output_file = "log_search_results.json";
    FILE* json_file = fopen(output_file, "w");
    if (!json_file) {
        std::cerr << "Error: Unable to open output file.\n";
        return;
    }

    fprintf(json_file, "%s\n", json_object_to_json_string_ext(json_array, JSON_C_TO_STRING_PRETTY));
    fclose(json_file);

    std::cout << "\nExported search results to " << output_file << "\n";
}
