#include "lexer.h"
#include "utils.h"

std::vector<Token> Lexer::lex() {
  for (char c : this->source) {
    this->buffer += c;

    // printf("C: %3c B: %s\n", c, this->buffer.c_str());

    if (this->in_string) {
      this->handle_string_char(c);
      continue;
    }

    if (this->in_number) {
      this->handle_number_char(c);
      if (this->in_number)
        continue;
    }

    bool handled_special_char = true;
    std::string prev_buffer = "";
    if (this->buffer.length() > 1)
      prev_buffer = this->buffer.substr(0, this->buffer.length() - 1);

    switch (c) {
    case QUOTE_DOUBLE:
    case QUOTE_SINGLE:
      this->handle_quote_char(c);
      handled_special_char = false;
      break;

    // If we've hit whitespace, or a new line, and are not
    // currently processing a string or number, then we can
    // safely assume the current contents of the buffer is
    // a word token.
    case ' ':
    case '\n':
      if (buffer != " ")
        this->push_token(TOKEN_WORD,
                         this->buffer.substr(0, this->buffer.length() - 1));
      this->clear_buffer();
      handled_special_char = false;
      break;

    case '(':
      this->push_token(TOKEN_PAREN_OPEN, "(");
      break;
    case ')':
      this->push_token(TOKEN_PAREN_CLOSE, ")");
      break;
    case '[':
      this->push_token(TOKEN_BRACKET_OPEN, "[");
      break;
    case ']':
      this->push_token(TOKEN_BRACKET_CLOSE, "]");
      break;
    case '{':
      this->push_token(TOKEN_BRACE_OPEN, "{");
      break;
    case '}':
      this->push_token(TOKEN_BRACE_CLOSE, "}");
      break;
    case '<':
      this->push_token(TOKEN_ANGLE_OPEN, "<");
      break;
    case '>':
      this->push_token(TOKEN_ANGLE_CLOSE, ">");
      break;

    case ',':
      this->push_token(TOKEN_COMMA, ",");
      break;
    case ';':
      this->push_token(TOKEN_SEMICOLON, ";");
      break;

    case '*':
      this->push_token(TOKEN_OPERATOR_MULTIPLY, "*");
      break;
    case '+':
      this->push_token(TOKEN_OPERATOR_PLUS, "+");
      break;
    case '-':
      this->push_token(TOKEN_OPERATOR_MINUS, "-");
      break;
    case '/':
      this->push_token(TOKEN_OPERATOR_DIVIDE, "/");
      break;
    case '%':
      this->push_token(TOKEN_OPERATOR_MODULO, "%");
      break;
    case '=':
      this->push_token(TOKEN_OPERATOR_EQUALS, "=");
      break;

    default:
      if (boom_utils::is_digit(c))
        this->in_number = true;

      handled_special_char = false;
      break;
    }

    if (handled_special_char && this->tokens.size() > 0) {
      Token last_added = this->tokens[this->tokens.size() - 1];
      this->tokens.pop_back();
      this->push_token(TOKEN_WORD, prev_buffer);
      this->push_token(last_added.id, last_added.value);
    }
  }

  if (this->in_string)
    this->handle_string_char(' ');
  if (this->in_number)
    this->handle_number_char(' ');

  this->tokens.push_back(Token(TOKEN_EOF, ""));

  return this->tokens;
}

bool Lexer::handle_string_char(char c) {
  if (c != this->in_string_type)
    return false;

  this->in_string = false;

  // The string is everything in the buffer except the
  // last character, which is the closing quote.
  this->push_token(TOKEN_STRING,
                   this->buffer.substr(0, this->buffer.length() - 1));
  return true;
}

bool Lexer::handle_number_char(char c) {
  if (boom_utils::is_digit(c) || c == '.') {
    return true;
  }

  // The number is everything in the buffer except the
  // last character, which is the character that caused
  // us to stop processing the number.
  this->push_token(TOKEN_NUMBER,
                   this->buffer.substr(0, this->buffer.length() - 1));
  this->buffer = c;
  this->in_number = false;
  return false;
}

void Lexer::clear_buffer() { this->buffer = ""; }

void Lexer::push_token(TokenIdentifier id, std::string value) {
  if (id == TOKEN_WORD && boom_utils::trim_string(value) == "") {
    this->clear_buffer();
    return;
  }

  this->tokens.push_back(Token(id, value));
  this->clear_buffer();
}

void Lexer::handle_quote_char(char c) {
  this->clear_buffer();
  this->in_string = true;
  if (c == '"')
    this->in_string_type = QUOTE_DOUBLE;
  else if (c == '\'')
    this->in_string_type = QUOTE_SINGLE;
}
