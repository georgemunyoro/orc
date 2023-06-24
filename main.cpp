#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include "ast.h"
#include "lexer.h"
#include "orc_llvm.h"
#include "parser.h"
#include "token.h"
#include "utils.h"

int main() {
  std::string source;
  boom_utils::read_source_file("./main.orc", &source);
  Lexer lexer(source);
  std::vector<Token> tokens = lexer.lex();

  Parser parser(&tokens);
  AST_Node *ast = parser.parse();

  OrcLLVM olm;
  // olm.exec(ast);
  olm.generate_binary(ast, "a.out");

  return 0;
}
