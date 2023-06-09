#pragma once

#include <iostream>
#include <string>
#include <vector>

enum AST_Node_Type {
  AST_NODE_UNKNOWN = 0,
  AST_NODE_EOF,
  AST_NODE_VARIABLE_DECLARATION,
  AST_NODE_VARIABLE_ASSIGNMENT,
  AST_NODE_BLOCK,
  AST_NODE_INTEGER_LITERAL,
  AST_NODE_VARIABLE_REFERENCE
};

class AST_Node {
public:
  virtual ~AST_Node() {}
  virtual void print(int indent = 0);
  virtual AST_Node_Type get_type() { return AST_NODE_UNKNOWN; }
};

class AST_VariableReference : public AST_Node {
public:
  AST_VariableReference(std::string name);
  ~AST_VariableReference();

  void print(int indent = 0);
  AST_Node_Type get_type() { return AST_NODE_VARIABLE_REFERENCE; }

  std::string name;
};

class AST_EOF : public AST_Node {
public:
  AST_EOF(){};
  ~AST_EOF(){};

  void print(int indent = 0);
  AST_Node_Type get_type() { return AST_NODE_EOF; }
};

class AST_VariableDeclaration : public AST_Node {
public:
  AST_VariableDeclaration(std::string name, std::string type, AST_Node *value);
  ~AST_VariableDeclaration();

  void print(int indent = 0);
  AST_Node_Type get_type() { return AST_NODE_VARIABLE_DECLARATION; }

  std::string name;
  std::string type;
  AST_Node *value;
};

class AST_VariableAssignment : public AST_Node {
public:
  AST_VariableAssignment(std::string name, AST_Node *value);
  ~AST_VariableAssignment();

  void print(int indent = 0);
  AST_Node_Type get_type() { return AST_NODE_VARIABLE_ASSIGNMENT; }

  std::string name;
  AST_Node *value;
};

class AST_Block : public AST_Node {
public:
  AST_Block();
  ~AST_Block();
  void print(int indent = 0);
  void add_node(AST_Node &node);

  AST_Node_Type get_type() { return AST_NODE_BLOCK; }

  std::vector<AST_Node *> nodes;
};

class AST_IntegerLiteral : public AST_Node {
public:
  AST_IntegerLiteral(std::string value);
  ~AST_IntegerLiteral();
  void print(int indent = 0);

  AST_Node_Type node_type = AST_NODE_INTEGER_LITERAL;

private:
  std::string value;
};

class AST_FloatLiteral : public AST_Node {
public:
  AST_FloatLiteral(std::string value);
  ~AST_FloatLiteral();
  void print(int indent = 0);

private:
  std::string value;
};

class AST_StringLiteral : public AST_Node {
public:
  AST_StringLiteral(std::string value);
  ~AST_StringLiteral();
  void print(int indent = 0);

private:
  std::string value;
};

class AST_FunctionArgument : public AST_Node {
public:
  AST_FunctionArgument(std::string name, std::string type);
  ~AST_FunctionArgument();
  void print(int indent = 0);

  std::string name;
  std::string type;
};

class AST_FunctionDefinition : public AST_Node {
public:
  AST_FunctionDefinition(std::string name,
                         std::vector<AST_FunctionArgument *> args,
                         AST_Block *body);
  ~AST_FunctionDefinition();
  void print(int indent = 0);

  std::string name;
  std::string return_type;
  std::vector<AST_FunctionArgument *> args;
  AST_Block *body;
};

class AST_FunctionCall : public AST_Node {
public:
  AST_FunctionCall(std::string name, std::vector<AST_Node *> args);
  ~AST_FunctionCall();
  void print(int indent = 0);

  std::string name;
  std::vector<AST_Node *> args;
};
