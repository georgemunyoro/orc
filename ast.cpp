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

llvm::Value *AST_EOF::codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) {
  return nullptr;
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

llvm::Value *AST_VariableDeclaration::codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) {
  return nullptr;
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

llvm::Value *AST_VariableReference::codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) {
  return nullptr;
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

llvm::Value *AST_VariableAssignment::codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) {
  return nullptr;
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

llvm::Value *AST_Block::codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) {
  return nullptr;
}


llvm::BasicBlock *AST_Block::codegen_block(
  std::string name,
  llvm::Function* fn,
  std::unique_ptr<llvm::LLVMContext> &context
) {
  return llvm::BasicBlock::Create(*context, name, fn);
}

/* AST_IntegerLiteral */

AST_IntegerLiteral::AST_IntegerLiteral(std::string value) {
  this->value = value;
}

AST_IntegerLiteral::~AST_IntegerLiteral() {}

void AST_IntegerLiteral::print(int indent) {
  printf("%sIntegerLiteral(%s)\n", boom_utils::indent_string(indent).c_str(),
         this->value.c_str());
}

llvm::Value *AST_IntegerLiteral::codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) {
  return builder->getInt32(42);
}

/* AST_FloatLiteral */

AST_FloatLiteral::AST_FloatLiteral(std::string value) { this->value = value; }

AST_FloatLiteral::~AST_FloatLiteral() {}

void AST_FloatLiteral::print(int indent) {
  printf("%sFloatLiteral(%s)\n", boom_utils::indent_string(indent).c_str(),
         this->value.c_str());
}

llvm::Value *AST_FloatLiteral::codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) {
  return nullptr;
}

/* AST_StringLiteral */

AST_StringLiteral::AST_StringLiteral(std::string value) { this->value = value; }

AST_StringLiteral::~AST_StringLiteral() {}

void AST_StringLiteral::print(int indent) {
  printf("%sStringLiteral(%s)\n", boom_utils::indent_string(indent).c_str(),
         this->value.c_str());
}

llvm::Value *AST_StringLiteral::codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) {
  return nullptr;
}

/* AST_FunctionDefinition */

AST_FunctionDefinition::AST_FunctionDefinition(
    std::string name, std::vector<AST_FunctionArgument *> args,
    AST_Block *body) {
  this->name = name;
  this->args = args;
  this->body = body;
}

AST_FunctionDefinition::~AST_FunctionDefinition() {}

void AST_FunctionDefinition::print(int indent) {
  printf("%sFunctionDefinition(\n%s%s\n",
         boom_utils::indent_string(indent).c_str(),
         boom_utils::indent_string(indent + 1).c_str(), this->name.c_str());
  printf("%sArguments(\n", boom_utils::indent_string(indent + 1).c_str());
  for (AST_FunctionArgument *arg : this->args)
    arg->print(indent + 2);
  printf("%s)\n", boom_utils::indent_string(indent + 1).c_str());
  this->body->print(indent + 1);
  printf("%s)\n", boom_utils::indent_string(indent).c_str());
}

llvm::Value *AST_FunctionDefinition::codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) {
  return nullptr;
}

llvm::Function *AST_FunctionDefinition::codegen_func(
  std::unique_ptr<llvm::IRBuilder<>> &builder,
  std::unique_ptr<llvm::Module> &module,
  std::unique_ptr<llvm::LLVMContext> &context
) {
  auto fn = module->getFunction(this->name);
  auto fn_type = llvm::FunctionType::get(builder->getVoidTy(), false);

  if (fn == nullptr) {
    fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, this->name, *module);
  }

  auto entry = this->body->codegen_block("entry", fn, context);
  builder->SetInsertPoint(entry);

  return fn;
}

/* AST_FunctionArgument */

AST_FunctionArgument::AST_FunctionArgument(std::string name, std::string type) {
  this->name = name;
  this->type = type;
}

AST_FunctionArgument::~AST_FunctionArgument() {}

void AST_FunctionArgument::print(int indent) {
  printf("%sFunctionArgument(\n%s%s\n%s%s\n",
         boom_utils::indent_string(indent).c_str(),
         boom_utils::indent_string(indent + 1).c_str(), this->name.c_str(),
         boom_utils::indent_string(indent + 1).c_str(), this->type.c_str());
  printf("%s)\n", boom_utils::indent_string(indent).c_str());
}

llvm::Value *AST_FunctionArgument::codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) {
  return nullptr;
}

/* AST_FunctionCall */

AST_FunctionCall::AST_FunctionCall(std::string name,
                                   std::vector<AST_Node *> args) {
  this->name = name;
  this->args = args;
}

AST_FunctionCall::~AST_FunctionCall() {}

void AST_FunctionCall::print(int indent) {
  printf("%sFunctionCall(\n%s%s\n", boom_utils::indent_string(indent).c_str(),
         boom_utils::indent_string(indent + 1).c_str(), this->name.c_str());
  printf("%sArguments(\n", boom_utils::indent_string(indent + 1).c_str());
  for (AST_Node *arg : this->args)
    arg->print(indent + 2);
  printf("%s)\n", boom_utils::indent_string(indent + 1).c_str());
  printf("%s)\n", boom_utils::indent_string(indent).c_str());
}

llvm::Value *AST_FunctionCall::codegen(std::unique_ptr<llvm::IRBuilder<>> &builder) {
    return nullptr;
}

