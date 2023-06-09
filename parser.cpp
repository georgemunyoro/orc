#include "parser.h"
#include "ast.h"
#include "token.h"
#include <cstdint>
#include <iostream>

Parser::Parser(std::vector<Token> *tokens) {
  this->tokens = tokens;
  this->cursor = 0;
  this->is_running = true;
}

Parser::~Parser() {}

AST_Node Parser::parse() {
  AST_Block ast;
  this->is_running = true;
  while (this->is_running)
    ast.add_node(this->parse_expr());
  return ast;
}

AST_Node Parser::parse_expr() {
  Token token = this->tokens->at(this->cursor);

  //   if (token.id == TOKEN_WORD) {
  //     if (token.value == "var") {
  //       std::string v_name;
  //       std::vector<Token> v_type_tokens;
  //       int eq_sign_pos, end_pos;

  //       for (int i = this->cursor + 1; i < this->tokens->size(); i++) {
  //         Token t = this->tokens->at(i);

  //         if (t.id == TOKEN_OPERATOR_EQUALS) {
  //           v_name = this->tokens->at(i - 1).value;
  //           eq_sign_pos = i;
  //           v_type_tokens =
  //               std::vector<Token>(this->tokens->begin() + this->cursor + 1,
  //                                  this->tokens->begin() + (i - 1));
  //         }

  //         if (t.id == TOKEN_SEMICOLON) {
  //           end_pos = i;
  //           break;
  //         }
  //       }

  //       std::string v_type = "";
  //       for (Token t : v_type_tokens) {
  //         v_type += t.value;
  //       }

  //       std::cout << "Found variable declaration: " << v_name << " of type "
  //                 << v_type << " with value "
  //                 << this->tokens->at(eq_sign_pos + 1).value << std::endl;

  //       AST_VariableDeclaration *v =
  //           new AST_VariableDeclaration(v_name, v_type, nullptr);

  //       ast.push_back(*v);
  //     }
  //   }

  //   if (token.id == TOKEN_NUMBER) {
  //     if (token.value.find(".") != std::string::npos) {
  //       AST_FloatLiteral *f_node = new AST_FloatLiteral(token.value);
  //       ast.push_back(*f_node);
  //     } else {
  //       AST_IntegerLiteral *i_node = new AST_IntegerLiteral(token.value);
  //       ast.push_back(*i_node);
  //     }
  //   }

  //   if (token.id == TOKEN_EOF) {
  //     this->is_running = false;
  //     break;
  //   }

  this->is_running = false;

  return AST_EOF();
}
