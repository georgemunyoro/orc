#pragma once

#include <cstddef>
#include <iostream>
#include <map>
#include <memory>
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
  AST_NODE_STRING_LITERAL,
  AST_NODE_VARIABLE_REFERENCE,
  AST_FUNCTION_DEFINITION,
  AST_FUNCTION_CALL,
  AST_BINARY_OPERATION,
  AST_FUNCTION_ARGUMENT,
  AST_CONDITIONAL,
  AST_LOOP
};

struct VariableDefinition {
  VariableDefinition() : v_value(nullptr), v_type(nullptr) {}

  VariableDefinition(llvm::Value *v_value, llvm::Type *v_type)
      : v_value(v_value), v_type(v_type) {}

  VariableDefinition(llvm::Value *v_value, llvm::Type *v_type, bool is_func_arg)
      : v_value(v_value), v_type(v_type), is_func_arg(is_func_arg) {}

  VariableDefinition(llvm::StructType *s_type)
      : s_type(s_type), is_struct(true) {}

  llvm::Value *v_value;
  llvm::Type *v_type;
  llvm::StructType *s_type;
  bool is_func_arg = false;
  bool is_struct = false;
  std::map<std::string, int> struct_field_map;
};

class AST_Node {
public:
  virtual ~AST_Node() {}
  virtual void print(int indent = 0);
  virtual AST_Node_Type get_type() { return AST_NODE_UNKNOWN; }
  virtual llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>>
              &variables) = 0;
};

class AST_VariableReference : public AST_Node {
public:
  AST_VariableReference(std::string name);
  ~AST_VariableReference();

  void print(int indent = 0) override;
  AST_Node_Type get_type() override { return AST_NODE_VARIABLE_REFERENCE; }
  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;

  std::string name;
};

class AST_EOF : public AST_Node {
public:
  AST_EOF(){};
  ~AST_EOF(){};

  void print(int indent = 0) override;
  AST_Node_Type get_type() override { return AST_NODE_EOF; }

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;
};

class AST_VariableDeclaration : public AST_Node {
public:
  AST_VariableDeclaration(std::string name, std::string type, AST_Node *value);
  ~AST_VariableDeclaration();

  void print(int indent = 0) override;
  AST_Node_Type get_type() override { return AST_NODE_VARIABLE_DECLARATION; }

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;
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

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;

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
  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;

  std::string block_type = "";
  std::string block_name = "";

  std::vector<AST_Node *> nodes;
  bool might_be_array = true;
};

class AST_IntegerLiteral : public AST_Node {
public:
  AST_IntegerLiteral(std::string value);
  ~AST_IntegerLiteral();
  void print(int indent = 0) override;

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;

  AST_Node_Type get_type() override { return AST_NODE_INTEGER_LITERAL; }

private:
  std::string value;
};

class AST_FloatLiteral : public AST_Node {
public:
  AST_FloatLiteral(std::string value);
  ~AST_FloatLiteral();
  void print(int indent = 0) override;

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;

private:
  std::string value;
};

class AST_StringLiteral : public AST_Node {
public:
  AST_StringLiteral(std::string value);
  ~AST_StringLiteral();
  void print(int indent = 0) override;

  AST_Node_Type get_type() override { return AST_NODE_STRING_LITERAL; }

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;

  std::string value;

private:
};

class AST_FunctionArgument : public AST_Node {
public:
  AST_FunctionArgument(std::string name, std::string type);
  ~AST_FunctionArgument();
  void print(int indent = 0) override;

  AST_Node_Type get_type() override { return AST_FUNCTION_ARGUMENT; }

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;

  std::string name;
  std::string type;
};

class AST_FunctionDefinition : public AST_Node {
public:
  AST_FunctionDefinition(std::string name,
                         std::vector<AST_FunctionArgument *> args,
                         std::string return_type, AST_Block *body);
  ~AST_FunctionDefinition();
  void print(int indent = 0) override;

  std::string name;
  std::string return_type;
  std::vector<AST_FunctionArgument *> args;
  AST_Block *body;
  AST_Node_Type get_type() override { return AST_FUNCTION_DEFINITION; }

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;
};

class AST_FunctionCall : public AST_Node {
public:
  AST_FunctionCall(std::string name, std::vector<AST_Node *> args);
  ~AST_FunctionCall();
  void print(int indent = 0) override;

  AST_Node_Type get_type() override { return AST_FUNCTION_CALL; }

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;

  std::string name;
  std::vector<AST_Node *> args;
};

class AST_BinaryOperation : public AST_Node {
public:
  AST_BinaryOperation(std::string op, AST_Node *left, AST_Node *right)
      : op(op), left(left), right(right) {}

  void print(int indent = 0) override;

  virtual AST_Node_Type get_type() override { return AST_BINARY_OPERATION; };

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;

  std::string op;
  AST_Node *left;
  AST_Node *right;
};

class AST_Conditional : public AST_Node {
public:
  AST_Conditional(AST_Block *condition, AST_Block *onTrue)
      : condition(condition), onTrue(onTrue), onFalse(new AST_Block()) {}

  AST_Conditional(AST_Block *condition, AST_Block *onTrue, AST_Block *onFalse)
      : condition(condition), onTrue(onTrue), onFalse(onFalse) {}

  virtual AST_Node_Type get_type() override { return AST_CONDITIONAL; };
  void print(int indent = 0) override;

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;

  AST_Block *condition;
  AST_Block *onTrue;
  AST_Block *onFalse;
};

class AST_Loop : public AST_Node {
public:
  AST_Loop(AST_Block *condition, AST_Block *expression)
      : condition(condition), expression(expression) {}

  void print(int indent = 0) override;
  virtual AST_Node_Type get_type() override { return AST_LOOP; };

  llvm::Value *
  codegen(std::unique_ptr<llvm::IRBuilder<>> &builder,
          std::unique_ptr<llvm::LLVMContext> &context,
          std::unique_ptr<llvm::Module> &module,
          std::unique_ptr<std::map<std::string, VariableDefinition>> &variables)
      override;

  AST_Block *condition;
  AST_Block *expression;
};

bool does_block_end_in_return(AST_Block *block);
llvm::Type *get_type_from_t_name(
    std::string &t_name, std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables);
