#include "json_export.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <json-c/json.h>
#include "utils.hpp"

extern char log_search_path[BUFFER_SIZE];

void export_search_results_to_json(const char *unused) {
    std::string egrep_args = get_user_input("\nRegEX / Text > ");
    if (egrep_args.empty() || !sanitize_input(egrep_args)) return;

    char cmd[BUFFER_SIZE * 2];
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    snprintf(cmd, sizeof(cmd), "%s | grep --color=never -i \"%s\"", find_cmd, egrep_args.c_str());
    FILE *proc = popen(cmd, "r");
    if (!proc) {
        std::cerr << "Error executing command.\n";
        return;
    }

    json_object *j_array = json_object_new_array();
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), proc)) {
        json_object *j_obj = json_object_new_object();
        json_object *j_msg = json_object_new_string(buffer);
        json_object_object_add(j_obj, "message", j_msg);
        json_object_array_add(j_array, j_obj);
    }
    pclose(proc);

    std::string json_file = get_user_input("\nEnter output JSON file name > ");
    if (json_file.empty() || !sanitize_input(json_file)) return;

    FILE *file = fopen(json_file.c_str(), "w");
    if (file) {
        fprintf(file, "%s", json_object_to_json_string_ext(j_array, JSON_C_TO_STRING_PRETTY));
        fclose(file);
        std::cout << "Exported to " << json_file << "\n";
    } else {
        std::cerr << "Failed to open file.\n";
    }

    json_object_put(j_array);
}
