#include <json-c/json.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "json_export.hpp"
#include "utils.hpp"

void export_search_results_to_json(const char *log_search_path) {
    std::string egrep_args = get_user_input("\nRegEX / Text > ");
    if (!sanitize_input(egrep_args)) return;

    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | egrep --color=never -i \"%s\"", find_cmd, egrep_args.c_str());

    FILE *proc = popen(cmd, "r");
    if (proc == nullptr) {
        perror("popen");
        return;
    }

    json_object *json_arr = json_object_new_array();
    char buffer[BUFFER_SIZE];
    bool has_entries = false;
    std::string all_entries;

    while (fgets(buffer, sizeof(buffer), proc)) {
        json_object *json_obj = json_object_new_object();
        json_object_object_add(json_obj, "log_entry", json_object_new_string(buffer));
        json_object_array_add(json_arr, json_obj);

        all_entries += buffer;
        has_entries = true;
    }

    pclose(proc);

    if (has_entries) {
        const char *output_file = "log_search_results.json";
        FILE *output = fopen(output_file, "w");
        if (output) {
            fputs(json_object_to_json_string(json_arr), output);
            fclose(output);
            std::cout << "\nExported search results to " << output_file << std::endl;
        } else {
            perror("fopen");
        }

        display_buffer_with_less(all_entries.c_str(), all_entries.size());
    } else {
        std::cout << "\nNo matching log entries found.\n";
    }

    json_object_put(json_arr);
    reset_menu_flag();
}
