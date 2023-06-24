#pragma once

#include <map>
#include <memory>

#include "ast.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

class OrcLLVM {
public:
  OrcLLVM(){};

  void exec(AST_Node *ast);
  void module_init();
  void generate_binary(std::string filename);

private:
  std::unique_ptr<std::map<std::string, VariableDefinition>> variables;
  std::unique_ptr<llvm::LLVMContext> llvm_ctx;
  std::unique_ptr<llvm::Module> llvm_mod;
  std::unique_ptr<llvm::IRBuilder<>> llvm_builder;
};
