#include "orc_llvm.h"

void OrcLLVM::exec(AST_Node *ast) {
  printf("\n--- Generated AST ---\n");
  ast->print();

  this->module_init();

  printf("\n\n--- DEBUG ---\n");

  // --

  AST_FunctionDefinition *main_func_node = (AST_FunctionDefinition*)ast;

  main_func_node->print();
  
  auto result = main_func_node->codegen_func(this->llvm_builder, this->llvm_mod, this->llvm_ctx);

  if (result == nullptr) {
    printf("NOT IMPLEMENTED %d\n", main_func_node->get_type());
    exit(1);
  }

  auto i32Result = this->llvm_builder->CreateIntCast(result,
                                                     this->llvm_builder->getInt32Ty(),
                                                     true);

  this->llvm_builder->CreateRet(i32Result);

  // --

  printf("\n\n--- Generated IR ---\n");
  this->llvm_mod->print(llvm::outs(), nullptr);
}

void OrcLLVM::module_init() {
  this->llvm_ctx = std::make_unique<llvm::LLVMContext>();
  this->llvm_mod = std::make_unique<llvm::Module>("OrcLLVM", *this->llvm_ctx);
  this->llvm_builder = std::make_unique<llvm::IRBuilder<>>(*this->llvm_ctx);
}

