#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "token.h"
#include "utils.h"
#include "orc_llvm.h"

int main() {
  std::string source;
  boom_utils::read_source_file("./main.orc", &source);
  Lexer lexer(source);
  std::vector<Token> tokens = lexer.lex();

  Parser parser(&tokens);
  AST_Node *ast = parser.parse();

  OrcLLVM olm;
  olm.exec(ast);

  return 0;
}
