#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "logs.h"
#include "json_export.h"
#include "utils.h"

#define ASCII_ART "\n" \
ANSI_COLOR_MAGENTA " 888                       888    888  .d88888b.   .d8888b. " ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888                       888    888 d88P\" \"Y88b d88P  Y88b" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888                       888    888 888     888 888    888" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888      .d88b.   .d88b.  8888888888 888     888 888       " ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888     d88\"\"88b d88P\"88b 888    888 888     888 888  88888" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888     888  888 888  888 888    888 888     888 888    888" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 888     Y88..88P Y88b 888 888    888 Y88b. .d88P Y88b  d88P" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA " 88888888 \"Y88P\"   \"Y88888 888    888  \"Y88888P\"   \"Y8888P88" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA "                       888" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA "                  Y8b d88P " ANSI_COLOR_RED " NO" ANSI_COLOR_LIGHT_GRAY "-nonsense digital forensics" ANSI_COLOR_RESET "\n" \
ANSI_COLOR_MAGENTA "                   \"Y88P\"" ANSI_COLOR_RESET "\n"

// Global search paths
char log_search_path[BUFFER_SIZE];

/**
 * Displays the main menu and handles user input.
 */
void main_menu() {
    char *option;
    while (1) {
        printf(ANSI_COLOR_GREEN ASCII_ART ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "(" ANSI_COLOR_LIGHT_GREEN "A" ANSI_COLOR_DARK ")" ANSI_COLOR_BLUE "uth\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "(" ANSI_COLOR_LIGHT_GREEN "E" ANSI_COLOR_DARK ")" ANSI_COLOR_BLUE "rrors\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "(" ANSI_COLOR_LIGHT_GREEN "L" ANSI_COLOR_DARK ")" ANSI_COLOR_BLUE "ive All\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "(" ANSI_COLOR_LIGHT_GREEN "N" ANSI_COLOR_DARK ")" ANSI_COLOR_BLUE "etwork\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "(" ANSI_COLOR_LIGHT_GREEN "R" ANSI_COLOR_DARK ")" ANSI_COLOR_BLUE "egex\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "(" ANSI_COLOR_LIGHT_GREEN "I" ANSI_COLOR_DARK ")" ANSI_COLOR_BLUE "P Search\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "(" ANSI_COLOR_LIGHT_GREEN "S" ANSI_COLOR_DARK ")" ANSI_COLOR_BLUE "et Log Paths\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "(" ANSI_COLOR_LIGHT_GREEN "J" ANSI_COLOR_DARK ")" ANSI_COLOR_BLUE "SON Export\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "(" ANSI_COLOR_LIGHT_GREEN "H" ANSI_COLOR_DARK ")" ANSI_COLOR_BLUE "elp\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "(" ANSI_COLOR_LIGHT_GREEN "Q" ANSI_COLOR_DARK ")" ANSI_COLOR_BLUE "uit\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_DARK "\n-" ANSI_COLOR_LIGHT_GRAY "> " ANSI_COLOR_RESET);

        option = readline(NULL);
        if (option == NULL) {
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        char opt = option[0];
        switch (opt) {
            case 'A':
            case 'a':
                live_auth_log(log_search_path);
                break;
            case 'E':
            case 'e':
                live_error_log(log_search_path);
                break;
            case 'L':
            case 'l':
                live_log(log_search_path);
                break;
            case 'N':
            case 'n':
                live_network_log(log_search_path);
                break;
            case 'R':
            case 'r':
                run_regex(log_search_path);
                break;
            case 'I':
            case 'i':
                search_ip(log_search_path);
                break;
            case 'S':
            case 's':
                edit_log_paths(log_search_path);
                break;
            case 'J':
            case 'j':
                export_search_results_to_json(log_search_path);
                break;
            case 'H':
            case 'h':
                display_help();
                break;
            case 'Q':
            case 'q':
                free(option);
                exit(0);
                break;
            default:
                printf(ANSI_COLOR_RED "Invalid option. Please try again.\n" ANSI_COLOR_RESET);
        }
        free(option);
    }
}

/**
 * Main entry point of the application.
 * @return Exit status code.
 */
int main() {
    // Load initial log paths from the config file
    load_log_paths(log_search_path, BUFFER_SIZE);

    signal(SIGINT, sigint_handler);
    main_menu();
    return 0;
}
