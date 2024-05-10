#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

void sigint_handler(int sig);
void reset_menu_flag();
void display_buffer_with_less(const char* buffer, size_t size);
std::string sanitize_input(const std::string& input);

#endif // UTILS_HPP
