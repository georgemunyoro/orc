
#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "token.h"

class Lexer {
public:
  Lexer(std::string source) {
    this->source = source;
    this->cursor = 0;
  }

  ~Lexer() = default;

  std::vector<Token> lex();

private:
  std::string source;
  int cursor;
  std::string buffer;

  bool in_number;

  bool in_string;
  QUOTE_TYPE in_string_type;

  std::vector<Token> tokens;

  void clear_buffer();

  bool handle_string_char(char c);
  bool handle_number_char(char c);
  void handle_quote_char(char c);

  void push_token(TokenIdentifier id, std::string value);
};
