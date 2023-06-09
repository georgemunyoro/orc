#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"


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
  virtual llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) = 0;
};

class AST_VariableReference : public AST_Node {
public:
  AST_VariableReference(std::string name);
  ~AST_VariableReference();

  void print(int indent = 0) override;
  AST_Node_Type get_type() override { return AST_NODE_VARIABLE_REFERENCE; }
  llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) override;

  std::string name;
};

class AST_EOF : public AST_Node {
public:
  AST_EOF(){};
  ~AST_EOF(){};

  void print(int indent = 0) override;
  AST_Node_Type get_type() override { return AST_NODE_EOF; }
  llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) override;
};

class AST_VariableDeclaration : public AST_Node {
public:
  AST_VariableDeclaration(std::string name, std::string type, AST_Node *value);
  ~AST_VariableDeclaration();

  void print(int indent = 0) override;
  AST_Node_Type get_type() override { return AST_NODE_VARIABLE_DECLARATION; }
  llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) override;

  std::string name;
  std::string type;
  AST_Node *value;
};

class AST_VariableAssignment : public AST_Node {
public:
  AST_VariableAssignment(std::string name, AST_Node *value);
  ~AST_VariableAssignment();

  void print(int indent = 0) override;
  AST_Node_Type get_type() override { return AST_NODE_VARIABLE_ASSIGNMENT; }
  llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) override;

  std::string name;
  AST_Node *value;
};

class AST_Block : public AST_Node {
public:
  AST_Block();
  ~AST_Block();
  void print(int indent = 0) override;
  void add_node(AST_Node &node);

  AST_Node_Type get_type() override { return AST_NODE_BLOCK; }
  llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) override;
  llvm::BasicBlock *codegen_block(
    std::string name,
    llvm::Function* fn,
    std::unique_ptr<llvm::LLVMContext> &context
  );

  // llvm::BasicBlock *codegen_func(std::unique_ptr<llvm::IRBuilder<>> &builder, std::unique_ptr<llvm::Module> &module);

  std::vector<AST_Node *> nodes;
};

class AST_IntegerLiteral : public AST_Node {
public:
  AST_IntegerLiteral(std::string value);
  ~AST_IntegerLiteral();
  void print(int indent = 0) override;

  AST_Node_Type node_type = AST_NODE_INTEGER_LITERAL;
  llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) override;

private:
  std::string value;
};

class AST_FloatLiteral : public AST_Node {
public:
  AST_FloatLiteral(std::string value);
  ~AST_FloatLiteral();
  void print(int indent = 0) override;
  llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) override;

private:
  std::string value;
};

class AST_StringLiteral : public AST_Node {
public:
  AST_StringLiteral(std::string value);
  ~AST_StringLiteral();
  void print(int indent = 0) override;
  llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) override;

private:
  std::string value;
};

class AST_FunctionArgument : public AST_Node {
public:
  AST_FunctionArgument(std::string name, std::string type);
  ~AST_FunctionArgument();
  void print(int indent = 0) override;
  llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) override;

  std::string name;
  std::string type;
};

class AST_FunctionDefinition : public AST_Node {
public:
  AST_FunctionDefinition(std::string name,
                         std::vector<AST_FunctionArgument *> args,
                         AST_Block *body);
  ~AST_FunctionDefinition();
  void print(int indent = 0) override;
  llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) override;
  llvm::Function *codegen_func(std::unique_ptr<llvm::IRBuilder<>> &builder,
                               std::unique_ptr<llvm::Module> &module,
                               std::unique_ptr<llvm::LLVMContext> &context);

  std::string name;
  std::string return_type;
  std::vector<AST_FunctionArgument *> args;
  AST_Block *body;
};

class AST_FunctionCall : public AST_Node {
public:
  AST_FunctionCall(std::string name, std::vector<AST_Node *> args);
  ~AST_FunctionCall();
  void print(int indent = 0) override;
  llvm::Value *codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) override;

  std::string name;
  std::vector<AST_Node *> args;
};
