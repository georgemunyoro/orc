#include "orc_llvm.h"
#include "ast.h"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <memory>

#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>
#include <llvm/MC/MCStreamer.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>

void OrcLLVM::exec(AST_Node *ast) {
  printf("\n--- Generated AST ---\n");
  ast->print();

  this->module_init();

  if (llvm_mod == nullptr || llvm_builder == nullptr || llvm_ctx == nullptr) {
    std::cout << "One of the LLVM objects was not properly initialized.\n";
    return;
  }

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

void OrcLLVM::generate_binary(AST_Node *ast, std::string filename) {
  this->exec(ast);

  std::string output_ir_str;
  llvm::raw_string_ostream rso(output_ir_str);
  this->llvm_mod->print(rso, nullptr);
  rso.flush();

  std::ofstream output_f_stream("out.ll");
  output_f_stream << output_ir_str;
  output_f_stream.close();

  std::system("llc -filetype=obj out.ll -o out.o");
  std::system("clang -no-pie out.o -o a.out");
}
