#include "ansi_colors.h"
#include "ascii_art.h"
#include "loghog_functions.h"

int main() {
    signal(SIGINT, sigint_handler);
    main_menu();
    return 0;
}
