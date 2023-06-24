#include "orc_llvm.h"
#include "ast.h"
#include <cassert>
#include <memory>

void OrcLLVM::exec(AST_Node *ast) {
  printf("\n--- Generated AST ---\n");
  ast->print();

  this->module_init();

  if (llvm_mod == nullptr || llvm_builder == nullptr || llvm_ctx == nullptr) {
    std::cout << "One of the LLVM objects was not properly initialized.\n";
    return;
  }

  // --

  printf("\n\n--- DEBUG ---\n");

  std::vector<llvm::Type *> printf_arg_types;
  printf_arg_types.push_back(
      llvm_builder->getInt8PtrTy()); // pointer to i8 for the format string

  llvm::ArrayRef<llvm::Type *> arg_types(printf_arg_types);
  llvm::FunctionType *printf_type =
      llvm::FunctionType::get(llvm_builder->getInt32Ty(), arg_types,
                              true); // the 'true' here means this function type
                                     // takes a variable number of arguments
  llvm::FunctionCallee printf_func =
      llvm_mod->getOrInsertFunction("printf", printf_type);

  for (AST_Node *i : ((AST_Block *)ast)->nodes) {
    i->codegen(this->llvm_builder, this->llvm_ctx, this->llvm_mod,
               this->variables);
  }

  // --

  printf("\n\n--- Generated IR ---\n");
  this->llvm_mod->print(llvm::outs(), nullptr);
}

void OrcLLVM::module_init() {
  this->llvm_ctx = std::make_unique<llvm::LLVMContext>();
  this->llvm_mod = std::make_unique<llvm::Module>("OrcLLVM", *this->llvm_ctx);
  this->llvm_builder = std::make_unique<llvm::IRBuilder<>>(*this->llvm_ctx);
  this->variables =
      std::make_unique<std::map<std::string, VariableDefinition>>();
}

void OrcLLVM::generate_binary(std::string filename) {
  std::cout << "Generating binary!" << std::endl;
}
