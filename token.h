#pragma once

#include <string>

enum TokenIdentifier {
  TOKEN_ID,
  TOKEN_NUMBER,
  TOKEN_STRING,
  TOKEN_WORD,
  TOKEN_OPERATOR,
  TOKEN_EOF,

  TOKEN_PAREN_OPEN,
  TOKEN_PAREN_CLOSE,
  TOKEN_BRACE_OPEN,
  TOKEN_BRACE_CLOSE,
  TOKEN_BRACKET_OPEN,
  TOKEN_BRACKET_CLOSE,

  TOKEN_COMMA,
  TOKEN_SEMICOLON,

  TOKEN_OPERATOR_EQUALS,

  TOKEN_OPERATOR_PLUS,
  TOKEN_OPERATOR_IS_EQUALS,
  TOKEN_OPERATOR_MINUS,
  TOKEN_OPERATOR_MULTIPLY,
  TOKEN_OPERATOR_DIVIDE,
  TOKEN_OPERATOR_MODULO,
  TOKEN_ANGLE_OPEN,
  TOKEN_ANGLE_CLOSE,
};

enum QUOTE_TYPE {
  QUOTE_SINGLE = '\'',
  QUOTE_DOUBLE = '"',
};

class Token {
public:
  TokenIdentifier id;
  std::string value;

  Token(TokenIdentifier id, std::string value) {
    this->id = id;
    this->value = value;
  }

  ~Token() = default;
};
