#pragma once

#include <fstream>
#include <iostream>

namespace boom_utils {
void read_source_file(std::string path, std::string *source);
bool is_digit(char c);
std::string trim_string(std::string str);
std::string indent_string(int indent);
} // namespace boom_utils
