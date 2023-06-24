#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <vector>

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

AST_FunctionDefinition *Parser::parse_function_definition() {
  ++this->cursor;
  std::string f_name = this->current_token()->value;

  ++this->cursor;
  std::vector<AST_FunctionArgument *> f_args;
  ++this->cursor;

  if (this->current_token()->id == TOKEN_PAREN_CLOSE) {
    this->cursor += 1;
  } else {
    for (;;) {

      AST_FunctionArgument *arg = new AST_FunctionArgument(
          this->current_token()->value, this->peek_next_token()->value);

      f_args.push_back(arg);

      if (this->tokens->at(this->cursor + 2).id == TOKEN_PAREN_CLOSE) {
        this->cursor += 3;
        break;
      }

      this->cursor += 3;
    }
  }

  std::string f_type = this->current_token()->value;
  ++this->cursor;

  AST_Node *f_block = this->parse_expr();

  if (f_block->get_type() != AST_NODE_BLOCK) {
    std::cout << "Error: Function block must be a block" << std::endl;
    exit(1);
  }

  return new AST_FunctionDefinition(f_name, f_args, f_type,
                                    (AST_Block *)f_block);
}

AST_FunctionCall *Parser::parse_function_call() {
  std::string f_name = this->current_token()->value;

  this->cursor += 1;
  // std::vector<AST_Node *> f_args = this->parse_expr();

  AST_Node *node = this->parse_expr();
  std::vector<AST_Node *> f_args;

  if (node->get_type() != AST_NODE_BLOCK) {
    printf("Error: Function arguments must be a block\n");
    exit(1);
  }

  AST_Block *block = (AST_Block *)node;

  for (AST_Node *n : block->nodes) {
    f_args.push_back(n);
  }

  return new AST_FunctionCall(f_name, f_args);
}

AST_VariableDeclaration *Parser::parse_variable_declaration() {
  std::string v_name;
  std::vector<Token> v_type_tokens;
  int eq_sign_pos, end_pos;

  for (size_t i = this->cursor + 1; i < this->tokens->size(); i++) {
    Token t = this->tokens->at(i);

    if (t.id == TOKEN_OPERATOR_EQUALS) {
      v_name = this->tokens->at(this->cursor + 1).value;
      eq_sign_pos = i;
      v_type_tokens =
          std::vector<Token>(this->tokens->begin() + this->cursor + 2,
                             this->tokens->begin() + (i));
    }

    if (t.id == TOKEN_SEMICOLON) {
      end_pos = i;
      break;
    }
  }

  std::string v_type = "";
  for (Token t : v_type_tokens) {
    std::cout << t.value << "\n" << std::endl;
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
  this->cursor += 1;
  return v;
}

AST_VariableReference *Parser::parse_variable_reference() {
  Token *token = this->current_token();
  this->cursor++;
  return new AST_VariableReference(token->value);
}

AST_BinaryOperation *
Parser::parse_binary_operation(std::vector<Token>::iterator op,
                               std::vector<Token>::iterator end_of_op) {
  AST_BinaryOperation *bin_op = new AST_BinaryOperation("", nullptr, nullptr);

  std::cout << "---\n";
  for (auto i = tokens->begin() + this->cursor - 1; i != end_of_op; ++i) {
    std::cout << i->value << std::endl;
  }
  std::cout << "---\n";

  return bin_op;
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
    } else if (token->value == "func") {
      return this->parse_function_definition();
    } else if (this->peek_next_token()->id == TOKEN_PAREN_OPEN) {
      return this->parse_function_call();
    } else {
      auto var_ref = this->parse_variable_reference();
      if (current_token()->id >= TOKEN_OPERATOR_PLUS) {
        AST_BinaryOperation *bin_op =
            new AST_BinaryOperation(current_token()->value, var_ref, nullptr);
        ++this->cursor;
        bin_op->right = this->parse_expr();
        return bin_op;
      }
      return var_ref;
    }
  }

  if (token->id == TOKEN_PAREN_OPEN) {
    AST_Block *block = new AST_Block();
    this->cursor++;
    for (;;) {
      AST_Node *node = this->parse_expr();
      block->add_node(*node);

      if (node->get_type() == AST_NODE_EOF)
        break;
    }
    return block;
  }

  if (token->id == TOKEN_COMMA || token->id == TOKEN_SEMICOLON) {
    this->cursor++;
    return this->parse_expr();
  }

  if (token->id == TOKEN_PAREN_CLOSE || token->id == TOKEN_BRACE_CLOSE) {
    this->cursor++;
    return new AST_EOF();
  }

  if (token->id == TOKEN_BRACE_OPEN) {
    AST_Block *block = new AST_Block();
    this->cursor++;
    for (;;) {
      AST_Node *node = this->parse_expr();
      block->add_node(*node);

      if (node->get_type() == AST_NODE_EOF)
        break;
    }
    return block;
  }

  if (token->id == TOKEN_NUMBER) {

    if (token->value.find(".") != std::string::npos) {
      AST_FloatLiteral *f_node = new AST_FloatLiteral(token->value);
      ++this->cursor;

      if (current_token()->id >= TOKEN_OPERATOR_PLUS) {
        AST_BinaryOperation *bin_op =
            new AST_BinaryOperation(current_token()->value, f_node, nullptr);
        ++this->cursor;
        bin_op->right = this->parse_expr();
        return bin_op;
      }

      return f_node;
    } else {
      AST_IntegerLiteral *i_node = new AST_IntegerLiteral(token->value);
      ++this->cursor;

      if (current_token()->id >= TOKEN_OPERATOR_PLUS) {
        AST_BinaryOperation *bin_op =
            new AST_BinaryOperation(current_token()->value, i_node, nullptr);
        ++this->cursor;
        bin_op->right = this->parse_expr();
        return bin_op;
      }

      return i_node;
    }
  }

  if (token->id == TOKEN_STRING) {
    AST_StringLiteral *s_node = new AST_StringLiteral(token->value);
    this->cursor++;
    return s_node;
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
