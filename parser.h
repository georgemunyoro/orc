#pragma once

#include <vector>

#include "ast.h"
#include "token.h"

class Parser {
public:
  Parser(std::vector<Token> *tokens);
  ~Parser();
  AST_Node parse();

private:
  uint32_t cursor;
  std::vector<Token> *tokens;
  bool is_running;

  AST_Node parse_expr();
};
