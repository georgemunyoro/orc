#pragma once

#include <string>
#include <vector>

class AST_Node {
public:
  virtual ~AST_Node() {}
  virtual void print();
};

class AST_EOF : public AST_Node {
public:
  AST_EOF();
  ~AST_EOF();
  void print();
};

class AST_VariableDeclaration : public AST_Node {
public:
  AST_VariableDeclaration(std::string name, std::string type, AST_Node value);
  ~AST_VariableDeclaration();
  void print();

private:
  std::string name;
  std::string type;
  AST_Node value;
};

class AST_Block : public AST_Node {
public:
  AST_Block();
  ~AST_Block();
  void print();
  void add_node(AST_Node node);

private:
  std::vector<AST_Node> nodes;
};

class AST_IntegerLiteral : public AST_Node {
public:
  AST_IntegerLiteral(std::string value);
  ~AST_IntegerLiteral();
  void print();

private:
  std::string value;
};

class AST_FloatLiteral : public AST_Node {
public:
  AST_FloatLiteral(std::string value);
  ~AST_FloatLiteral();
  void print();

private:
  std::string value;
};

class AST_StringLiteral : public AST_Node {
public:
  AST_StringLiteral(std::string value);
  ~AST_StringLiteral();
  void print();

private:
  std::string value;
};
