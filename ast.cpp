#include <iostream>
#include <vector>

#include "ast.h"

/* AST_Node */

void AST_Node::print() {}

/* AST_EOF */

AST_EOF::AST_EOF() {}
AST_EOF::~AST_EOF() {}

void AST_EOF::print() { std::cout << "EOF" << std::endl; }

/* AST_VariableDeclaration */

AST_VariableDeclaration::AST_VariableDeclaration(std::string name,
                                                 std::string type,
                                                 AST_Node value) {
  this->name = name;
  this->type = type;
  this->value = value;
}

AST_VariableDeclaration::~AST_VariableDeclaration() {}

void AST_VariableDeclaration::print() {
  std::cout << "VariableDeclaration: " << this->name << " " << this->type
            << std::endl;
  this->value.print();
}

/* AST_Block */

AST_Block::AST_Block() {}
AST_Block::~AST_Block() {}

void AST_Block::print() {
  std::cout << "Block:" << std::endl;
  for (AST_Node node : this->nodes)
    node.print();
}

void AST_Block::add_node(AST_Node node) { this->nodes.push_back(node); }
