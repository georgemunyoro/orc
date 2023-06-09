#include "utils.h"

namespace boom_utils {
void read_source_file(std::string path, std::string *source) {
  std::ifstream source_file;
  source_file.open(path);
  if (source_file.is_open()) {
    std::string line;
    while (getline(source_file, line))
      *source += line + "\n";

    source_file.close();
  }
}

bool is_digit(char c) { return c >= '0' && c <= '9'; }

std::string trim_string(std::string str) {
  int first_non_whitespace = str.find_first_not_of(" \t\n");
  int last_non_whitespace = str.find_last_not_of(" \t\n");
  if (first_non_whitespace == std::string::npos)
    return "";
  return str.substr(first_non_whitespace,
                    last_non_whitespace - first_non_whitespace + 1);
}
} // namespace boom_utils
