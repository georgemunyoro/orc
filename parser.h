#pragma once

#include <vector>

#include "ast.h"
#include "token.h"

class Parser {
public:
  Parser(std::vector<Token> *tokens);
  ~Parser();
  AST_Node *parse();

private:
  uint32_t cursor;
  std::vector<Token> *tokens;
  bool is_running;

  AST_Node *parse_expr();

  AST_VariableDeclaration *parse_variable_declaration();
  AST_VariableAssignment *parse_variable_assignment();
  AST_VariableReference *parse_variable_reference();
  AST_FunctionDefinition *parse_function_definition();
  AST_FunctionCall *parse_function_call();
  AST_BinaryOperation *
  parse_binary_operation(std::vector<Token>::iterator op,
                         std::vector<Token>::iterator end_of_op);

  Token *current_token();
  Token *peek_next_token();
  Token *peek_prev_token();
};
