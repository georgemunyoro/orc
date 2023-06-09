#include <iostream>
#include <vector>

#include "ast.h"
#include "utils.h"

/* AST_Node */

void AST_Node::print(int indent) {
  std::cout << boom_utils::indent_string(indent) << "AST_Node" << std::endl;
}

/* AST_EOF */

void AST_EOF::print(int indent) {
  std::cout << boom_utils::indent_string(indent) << "EOF" << std::endl;
}

/* AST_VariableDeclaration */

AST_VariableDeclaration::AST_VariableDeclaration(std::string name,
                                                 std::string type,
                                                 AST_Node *value) {
  this->name = name;
  this->type = type;
  this->value = value;
}

AST_VariableDeclaration::~AST_VariableDeclaration() {}

void AST_VariableDeclaration::print(int indent) {
  printf("%sVariableDeclaration(\n%s%s\n%s%s\n",
         boom_utils::indent_string(indent).c_str(),
         boom_utils::indent_string(indent + 1).c_str(), this->name.c_str(),
         boom_utils::indent_string(indent + 1).c_str(), this->type.c_str());
  this->value->print(indent + 1);
  printf("%s)\n", boom_utils::indent_string(indent).c_str());
}

/* AST_VariableReference */

AST_VariableReference::AST_VariableReference(std::string name) {
  this->name = name;
}

AST_VariableReference::~AST_VariableReference() {}

void AST_VariableReference::print(int indent) {
  printf("%sVariableReference(%s)\n", boom_utils::indent_string(indent).c_str(),
         this->name.c_str());
}

/* AST_VariableAssignment */

AST_VariableAssignment::AST_VariableAssignment(std::string name,
                                               AST_Node *value) {
  this->name = name;
  this->value = value;
}

AST_VariableAssignment::~AST_VariableAssignment() {}

void AST_VariableAssignment::print(int indent) {
  printf("%sVariableAssignment(\n%s%s\n",
         boom_utils::indent_string(indent).c_str(),
         boom_utils::indent_string(indent + 1).c_str(), this->name.c_str());
  this->value->print(indent + 1);
  printf("%s)\n", boom_utils::indent_string(indent).c_str());
}

/* AST_Block */

AST_Block::AST_Block() {}
AST_Block::~AST_Block() {}

void AST_Block::print(int indent) {
  if (this->nodes.size() == 0) {
    printf("%sBlock{}\n", boom_utils::indent_string(indent).c_str());
    return;
  }

  printf("%sBlock{\n", boom_utils::indent_string(indent).c_str());
  for (AST_Node *node : this->nodes)
    node->print(indent + 1);
  printf("%s}\n", boom_utils::indent_string(indent).c_str());
}

void AST_Block::add_node(AST_Node &node) { this->nodes.push_back(&node); }

/* AST_IntegerLiteral */

AST_IntegerLiteral::AST_IntegerLiteral(std::string value) {
  this->value = value;
}

AST_IntegerLiteral::~AST_IntegerLiteral() {}

void AST_IntegerLiteral::print(int indent) {
  printf("%sIntegerLiteral(%s)\n", boom_utils::indent_string(indent).c_str(),
         this->value.c_str());
}

/* AST_FloatLiteral */

AST_FloatLiteral::AST_FloatLiteral(std::string value) { this->value = value; }

AST_FloatLiteral::~AST_FloatLiteral() {}

void AST_FloatLiteral::print(int indent) {
  printf("%sFloatLiteral(%s)\n", boom_utils::indent_string(indent).c_str(),
         this->value.c_str());
}

/* AST_StringLiteral */

AST_StringLiteral::AST_StringLiteral(std::string value) { this->value = value; }

AST_StringLiteral::~AST_StringLiteral() {}

void AST_StringLiteral::print(int indent) {
  printf("%sStringLiteral(%s)\n", boom_utils::indent_string(indent).c_str(),
         this->value.c_str());
}
