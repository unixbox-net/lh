#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include "logs.h"
#include "utils.h"

/**
 * Prompts the user to enter a regex or pattern and exports the results to a JSON file.
 * @param log_search_path Paths to search for logs.
 */
void export_search_results_to_json(const char *log_search_path) {
    char *egrep_args = get_user_input("\nRegEX / Text > ");
    if (!sanitize_input(egrep_args)) {
        free(egrep_args);
        return;
    }

    char cmd[BUFFER_SIZE * 2]; // Ensure sufficient buffer size
    char find_cmd[BUFFER_SIZE];
    find_logs_command(find_cmd, sizeof(find_cmd), log_search_path);
    if (snprintf(cmd, sizeof(cmd), "%s | egrep --color=never -i \"%s\"", find_cmd, egrep_args) >= sizeof(cmd)) {
        printf(ANSI_COLOR_RED "Command is too long. Please refine your search.\n" ANSI_COLOR_RESET);
        free(egrep_args);
        return;
    }

    FILE *proc = popen(cmd, "r");
    if (!proc) {
        perror("popen");
        free(egrep_args);
        return;
    }

    json_object *json_arr = json_object_new_array();
    char buffer[BUFFER_SIZE];
    int has_entries = 0;

    while (fgets(buffer, sizeof(buffer), proc)) {
        json_object *json_obj = json_object_new_object();
        json_object_object_add(json_obj, "log_entry", json_object_new_string(buffer));
        json_object_array_add(json_arr, json_obj);
        has_entries = 1;
    }

    pclose(proc);

    if (has_entries) {
        const char *output_file = "log_search_results.json";
        FILE *output = fopen(output_file, "w");
        if (output) {
            fputs(json_object_to_json_string_ext(json_arr, JSON_C_TO_STRING_PRETTY), output);
            fclose(output);
            printf("\nExported search results to %s\n", output_file);

            // Print JSON directly using `less`
            display_buffer_with_less(json_object_to_json_string_ext(json_arr, JSON_C_TO_STRING_PRETTY), strlen(json_object_to_json_string_ext(json_arr, JSON_C_TO_STRING_PRETTY)));
        } else {
            perror("fopen");
        }
    } else {
        printf("\nNo matching log entries found.\n");
    }

    json_object_put(json_arr);
    free(egrep_args);
}
