#include <cstdint>
#include <cstdio>
#include <iostream>

#include "ast.h"
#include "parser.h"
#include "token.h"

Parser::Parser(std::vector<Token> *tokens) {
  this->tokens = tokens;
  this->cursor = 0;
  this->is_running = true;
}

Parser::~Parser() {}

AST_Node *Parser::parse() {
  AST_Block *ast = new AST_Block();

  for (;;) {
    AST_Node *node = this->parse_expr();
    ast->add_node(*node);

    if (node->get_type() == AST_NODE_EOF)
      break;
  }

  return ast;
}

AST_VariableDeclaration *Parser::parse_variable_declaration() {
  std::string v_name;
  std::vector<Token> v_type_tokens;
  int eq_sign_pos, end_pos;

  for (int i = this->cursor + 1; i < this->tokens->size(); i++) {
    Token t = this->tokens->at(i);

    if (t.id == TOKEN_OPERATOR_EQUALS) {
      v_name = this->tokens->at(i - 1).value;
      eq_sign_pos = i;
      v_type_tokens =
          std::vector<Token>(this->tokens->begin() + this->cursor + 1,
                             this->tokens->begin() + (i - 1));
    }

    if (t.id == TOKEN_SEMICOLON) {
      end_pos = i;
      break;
    }
  }

  std::string v_type = "";
  for (Token t : v_type_tokens) {
    v_type += t.value;
  }

  this->cursor = eq_sign_pos + 1;

  AST_VariableDeclaration *v =
      new AST_VariableDeclaration(v_name, v_type, this->parse_expr());

  this->cursor = end_pos + 1;

  return v;
}

AST_VariableAssignment *Parser::parse_variable_assignment() {
  Token *token = this->current_token();
  std::string v_name = token->value;
  this->cursor += 2;
  AST_VariableAssignment *v =
      new AST_VariableAssignment(token->value, this->parse_expr());
  this->cursor += 2;
  return v;
}

AST_VariableReference *Parser::parse_variable_reference() {
  Token *token = this->current_token();
  this->cursor++;
  return new AST_VariableReference(token->value);
}

AST_Node *Parser::parse_expr() {
  Token *token = this->current_token();

  if (token == nullptr) {
    AST_EOF *eof = new AST_EOF();
    this->is_running = false;
    return eof;
  }

  if (token->id == TOKEN_WORD) {
    if (token->value == "var") {
      return this->parse_variable_declaration();
    } else if (this->peek_next_token()->id == TOKEN_OPERATOR_EQUALS) {
      return this->parse_variable_assignment();
    } else {
      return this->parse_variable_reference();
    }
  }

  if (token->id == TOKEN_NUMBER) {
    if (token->value.find(".") != std::string::npos) {
      AST_FloatLiteral *f_node = new AST_FloatLiteral(token->value);
      return f_node;
    } else {
      AST_IntegerLiteral *i_node = new AST_IntegerLiteral(token->value);
      return i_node;
    }
  }

  AST_EOF *eof = new AST_EOF();
  this->is_running = false;
  return eof;
}

Token *Parser::current_token() {
  if (this->cursor >= this->tokens->size())
    return nullptr;
  return &this->tokens->at(this->cursor);
}

Token *Parser::peek_next_token() {
  if (this->cursor + 1 >= this->tokens->size())
    return nullptr;
  return &this->tokens->at(this->cursor + 1);
}

Token *Parser::peek_prev_token() {
  if (this->cursor - 1 < 0)
    return nullptr;
  return &this->tokens->at(this->cursor - 1);
}
