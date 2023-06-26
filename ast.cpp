#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/InstrTypes.h>
#include <string>
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

llvm::Value *AST_EOF::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {
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

llvm::Value *AST_VariableDeclaration::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {

  auto val = this->value->codegen(builder, context, module, variables);
  // if (val->getType()->isArrayTy()) {
  //   std::cout << this->name << " IS AN ARRAY" << std::endl;
  //   (*variables)[this->name] = VariableDefinition();
  //   (*variables)[this->name].v_value = val;
  //   (*variables)[this->name].v_type = val->getType();
  // }

  /*
    Struct variable declaration
  */
  if (this->value->get_type() == AST_FUNCTION_CALL) {
    AST_FunctionCall *f_call = (AST_FunctionCall *)this->value;

    if (val->getType()->getPointerElementType()->isStructTy()) {
      llvm::Value *s_instance = val;

      std::string s_name = boom_utils::trim_string(s_instance->getType()
                                                       ->getPointerElementType()
                                                       ->getStructName()
                                                       .str());

      (*variables)[this->name] = VariableDefinition();
      (*variables)[this->name].v_value = s_instance;
      (*variables)[this->name].s_type = (*variables)[s_name].s_type;

      return s_instance;
    }

    if ((*variables).contains(f_call->name)) {
      VariableDefinition stored_def = (*variables)[f_call->name];

      if (stored_def.is_struct) {

        llvm::Value *s_instance = val;

        (*variables)[this->name] = VariableDefinition();
        (*variables)[this->name].v_value = s_instance;
        (*variables)[this->name].s_type = (*variables)[f_call->name].s_type;

        return s_instance;
      }
    } else {
      return this->value->codegen(builder, context, module, variables);
    }
  }

  if (this->value->get_type() == AST_NODE_BLOCK) {

    AST_Block *block = ((AST_Block *)this->value);

    llvm::Value *val =
        this->value->codegen(builder, context, module, variables);

    (*variables)[this->name] = VariableDefinition(val, val->getType());
    return val;
  }

  if (this->value->get_type() == AST_NODE_INTEGER_LITERAL ||
      this->value->get_type() == AST_NODE_STRING_LITERAL) {

    llvm::Value *val =
        this->value->codegen(builder, context, module, variables);

    /*
      Strings are handled differently from other variable types. Instead of
      creating a local variable and storing that, we create a global string
      pointer and store that instead, this makes it easier to work with
      strings in function calls.

      TODO: There is definitely a more correct way to this.
    */
    if (this->value->get_type() == AST_NODE_STRING_LITERAL) {
      std::string str_value = ((AST_StringLiteral *)this->value)->value;
      llvm::Type *stringType = llvm::ArrayType::get(
          llvm::IntegerType::get(*context, 8), str_value.length() + 1);
      llvm::Value *global_str_ptr = builder->CreateGlobalStringPtr(str_value);

      (*variables)[this->name] = VariableDefinition(
          global_str_ptr, llvm::Type::getInt8PtrTy(*context));

      return global_str_ptr;
    }

    llvm::AllocaInst *alloca =
        builder->CreateAlloca(val->getType(), 0, this->name);
    (*variables)[this->name] = VariableDefinition(alloca, val->getType());
    return builder->CreateStore(val, alloca);
  }

  if (this->value->get_type() == AST_NODE_VARIABLE_REFERENCE) {
    /*
      Variable assignment is done by value, not reference
    */
    AST_VariableReference *varRef = (AST_VariableReference *)this->value;
    (*variables)[this->name] = (*variables)[varRef->name];
    return (*variables)[this->name].v_value;
  }

  llvm::AllocaInst *alloca =
      builder->CreateAlloca(val->getType(), 0, this->name);
  (*variables)[this->name] = VariableDefinition(alloca, val->getType());
  return builder->CreateStore(val, alloca);
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

llvm::Value *AST_VariableReference::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {

  VariableDefinition var_def = (*variables)[this->name];

  if (var_def.s_type != nullptr && var_def.v_type == nullptr &&
      !var_def.is_struct) {
    return var_def.v_value;
  }

  if (var_def.v_type->isPointerTy() || var_def.is_func_arg ||
      var_def.v_type->isArrayTy()) {

    return var_def.v_value;
  }

  llvm::Value *varRef = var_def.v_value;

  if (varRef == nullptr) {
    std::cout << "Variable `" << this->name
              << "` not found in symbols table!\n";
    exit(1);
  }

  llvm::AllocaInst *ptr = llvm::dyn_cast<llvm::AllocaInst>(varRef);

  if (ptr == nullptr) {
    std::cout << "Error! referencing `" << this->name
              << "` which is undefined.\n";
    exit(1);
  }

  llvm::LoadInst *loadedValue =
      builder->CreateLoad(ptr->getAllocatedType(), ptr);
  return loadedValue;
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

llvm::Value *AST_VariableAssignment::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {

  // Check if the variable exists
  if ((*variables).count(this->name) == 0) {
    printf("Error! Cannot assign to variable `%s` that was not previously "
           "defined.",
           this->name.c_str());
    exit(1);
  }

  VariableDefinition varDef = (*variables)[this->name];
  llvm::Value *val = this->value->codegen(builder, context, module, variables);

  if (val->getType()->getPointerElementType()->isArrayTy() &&
      varDef.v_type->getPointerElementType()->isArrayTy()) {

    if (val->getType()->getPointerElementType()->getArrayElementType() ==
        varDef.v_type->getPointerElementType()->getArrayElementType()) {
      printf("\n----v----\n");

      printf("\n----v----\n");

      auto x = builder->CreateStore(val, (*variables)[this->name].v_value);
      (*variables)[this->name] = VariableDefinition(val, val->getType());

      return x;
    }
  }

  if (val->getType() != varDef.v_type) {
    printf("Error! Attempted to assign incompatible type to variable `%s`",
           this->name.c_str());
    exit(1);
  }

  return builder->CreateStore(val, (*variables)[this->name].v_value);
}

/* AST_Block */

AST_Block::AST_Block() {}
AST_Block::~AST_Block() {}

void AST_Block::print(int indent) {
  if (this->nodes.size() == 0) {
    printf("%sBlock{}\n", boom_utils::indent_string(indent).c_str());
    return;
  }

  printf("%sBlock[name=%s, type=%s]{\n",
         boom_utils::indent_string(indent).c_str(), this->block_name.c_str(),
         this->block_type.c_str());
  for (AST_Node *node : this->nodes)
    node->print(indent + 1);
  printf("%s}\n", boom_utils::indent_string(indent).c_str());
}

void AST_Block::add_node(AST_Node &node) { this->nodes.push_back(&node); }

llvm::Value *AST_Block::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {

  bool is_array = true;
  llvm::Type *arr_element_type = nullptr;
  std::vector<llvm::Value *> values;

  for (auto n : this->nodes) {
    if (n->get_type() == AST_FUNCTION_ARGUMENT || !this->might_be_array) {
      is_array = false;
      break;
    }

    if (n->get_type() == AST_NODE_EOF)
      break;

    auto v = n->codegen(builder, context, module, variables);
    arr_element_type = v->getType();

    if (v->getType()->isVoidTy() || !(v->getType()->isIntegerTy()) ||
        (v->getType() != arr_element_type))
      is_array = false;

    values.push_back(v);
  }

  if (is_array) {
    llvm::ArrayType *array_type =
        llvm::ArrayType::get(arr_element_type, values.size());
    llvm::Value *array = builder->CreateAlloca(array_type, 0, "array");

    for (int i = 0; i < values.size(); ++i) {
      llvm::Value *ptr_to_element_at_i = builder->CreateGEP(
          array_type, array, {builder->getInt32(0), builder->getInt32(i)});

      builder->CreateStore(values.at(i), ptr_to_element_at_i);
    }

    return array;
  }

  if (this->block_type == "struct") {
    std::vector<llvm::Type *> structFields;

    std::map<std::string, int> s_field_map;

    for (int i = 0; i < this->nodes.size(); ++i) {
      AST_FunctionArgument *field_arg =
          (AST_FunctionArgument *)this->nodes.at(i);
      s_field_map.insert({field_arg->name, i});
      structFields.push_back(
          get_type_from_t_name(field_arg->type, context, variables));
    }

    llvm::StructType *structType =
        llvm::StructType::create(*context, structFields, this->block_name);

    (*variables)[this->block_name] = VariableDefinition(structType);
    (*variables)[this->block_name].struct_field_map = s_field_map;

    return nullptr;
  }

  llvm::Value *last = nullptr;

  for (auto n : this->nodes) {
    if (n->get_type() == AST_NODE_EOF)
      continue;
    last = n->codegen(builder, context, module, variables);
  }

  return last;
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

llvm::Value *AST_IntegerLiteral::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {
  return builder->getInt32(std::stoi(this->value));
}

/* AST_FloatLiteral */

AST_FloatLiteral::AST_FloatLiteral(std::string value) { this->value = value; }

AST_FloatLiteral::~AST_FloatLiteral() {}

void AST_FloatLiteral::print(int indent) {
  printf("%sFloatLiteral(%s)\n", boom_utils::indent_string(indent).c_str(),
         this->value.c_str());
}

llvm::Value *AST_FloatLiteral::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {
  return nullptr;
}

/* AST_StringLiteral */

AST_StringLiteral::AST_StringLiteral(std::string value) { this->value = value; }

AST_StringLiteral::~AST_StringLiteral() {}

void AST_StringLiteral::print(int indent) {
  printf("%sStringLiteral(%s)\n", boom_utils::indent_string(indent).c_str(),
         boom_utils::trim_string(this->value).c_str());
}

llvm::Value *AST_StringLiteral::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {
  return llvm::ConstantDataArray::getString(*context, this->value);
}

/* AST_FunctionDefinition */

AST_FunctionDefinition::AST_FunctionDefinition(
    std::string name, std::vector<AST_FunctionArgument *> args,
    std::string return_type, AST_Block *body) {
  this->name = name;
  this->args = args;
  this->body = body;
  this->return_type = return_type;
}

AST_FunctionDefinition::~AST_FunctionDefinition() {}

void AST_FunctionDefinition::print(int indent) {
  std::cout << boom_utils::indent_string(indent) << "FunctionDefinition<"
            << this->return_type << ">(\n"
            << boom_utils::indent_string(indent + 1) << this->name << "\n";
  std::cout << boom_utils::indent_string(indent + 1) << "Arguments(\n";
  for (AST_FunctionArgument *arg : this->args)
    arg->print(indent + 2);
  std::cout << boom_utils::indent_string(indent + 1) << ")\n";
  this->body->print(indent + 1);
  std::cout << boom_utils::indent_string(indent) << ")\n";
}

llvm::Type *get_type_from_t_name(
    std::string &t_name, std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {
  if (t_name == "int")
    return llvm::Type::getInt32Ty(*context);

  if (t_name == "string")
    return llvm::Type::getInt8PtrTy(*context);

  if (variables->contains(t_name)) {
    return (*variables)[t_name].s_type;
  }

  return llvm::Type::getVoidTy(*context);
}

llvm::Value *AST_FunctionDefinition::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {

  std::vector<llvm::Type *> func_arg_types;
  for (int i = 0; i < args.size(); ++i) {
    auto arg = this->args.at(i);
    if (arg->type == "string") {
      func_arg_types.push_back(builder->getInt8PtrTy());
      (*variables)[arg->name] =
          VariableDefinition(nullptr, builder->getInt8PtrTy(), true);
    } else if (arg->type == "int") {
      func_arg_types.push_back(builder->getInt32Ty());
      (*variables)[arg->name] =
          VariableDefinition(nullptr, builder->getInt8PtrTy(), true);
    } else {
      auto t = get_type_from_t_name(arg->type, context, variables);
      func_arg_types.push_back(t->getPointerTo());
      t->print(llvm::outs());
      (*variables)[arg->name] =
          VariableDefinition(nullptr, t->getPointerTo(), true);
    }
  }

  llvm::Type *result_type =
      (variables->contains(this->return_type))
          ? ((*variables)[this->return_type].s_type->getPointerTo())
          : (get_type_from_t_name(this->return_type, context, variables));

  llvm ::FunctionType *funcType =
      llvm::FunctionType::get(result_type, func_arg_types, false);

  llvm::Function *func = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, this->name, *module);

  if (func == nullptr) {
    std::cout << "Error ocurred while defining function `" << this->name
              << "`\n";
    exit(1);
  }

  for (int i = 0; i < args.size(); ++i) {
    auto arg = this->args.at(i);
    (*variables)[arg->name].v_value = func->getArg(i);
  }

  llvm::BasicBlock *func_block =
      llvm::BasicBlock::Create(*context, "entry", func);
  builder->SetInsertPoint(func_block);

  for (auto body_node : this->body->nodes) {
    body_node->codegen(builder, context, module, variables);
  }

  if (this->return_type == "void") {
    builder->CreateRetVoid();
  }

  builder->ClearInsertionPoint();

  return nullptr;
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

llvm::Value *AST_FunctionArgument::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {
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

llvm::Value *AST_FunctionCall::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {

  if (variables->count(this->name) > 0) {
    VariableDefinition stored_ref = (*variables)[this->name];

    /*
      Struct initialization
    */
    if (stored_ref.is_struct) {
      VariableDefinition s_type = stored_ref;

      if (s_type.s_type == nullptr) {
        printf("Error while initializing struct `%s`\n", this->name.c_str());
        exit(1);
      }

      llvm::Value *s_instance =
          builder->CreateAlloca(s_type.s_type, 0, this->name + "_struct");

      std::vector<llvm::Value *> indices(2);
      indices[0] = builder->getInt32(0);

      for (int i = 0; i < this->args.size() - 1; ++i) {
        AST_Node *curr_arg = this->args.at(i);
        indices[1] = builder->getInt32(i);

        llvm::Value *ptr =
            builder->CreateGEP(s_instance->getType()->getPointerElementType(),
                               s_instance, indices);

        builder->CreateStore(
            curr_arg->codegen(builder, context, module, variables), ptr);
      }

      return s_instance;
    }
  }

  /*
    Return statements
  */
  if (this->name == "return") {
    /*
      EOF is part of the function arguments, therefore when we have no arguments
      provided e.g. call_function(), it is represented as having a single EOF
      argument in the AST.
    */
    if (this->args.size() == 1) {
      return builder->CreateRetVoid();
    } else {

      if (this->args.size() > 2) {
        std::cout
            << "Error! Cannot return more than 1 value from a function.\n";
        exit(1);
      }

      return builder->CreateRet(
          this->args.at(0)->codegen(builder, context, module, variables));
    }
  }

  llvm::Function *func = module->getFunction(this->name);

  if (func == nullptr) {
    std::cout << "Error! Function `" << this->name << "` not found!\n";
    exit(1);
  }

  if (this->name == "printf") {
    llvm::Value *val =
        this->args.at(0)->codegen(builder, context, module, variables);

    bool is_pointer_to_i8 =
        val->getType()->isPointerTy() &&
        val->getType()->getPointerElementType()->isIntegerTy(8);

    bool is_array_of_i8 = val->getType()->isArrayTy() &&
                          val->getType()->getArrayElementType()->isIntegerTy(8);

    if (!is_pointer_to_i8 && !is_array_of_i8) {
      std::cout << "Error while calling `printf`! First argument must always "
                   "be the format string.\n";
      exit(1);
    }

    llvm::Value *format_str = builder->CreateGlobalStringPtr(
        cast<llvm::ConstantDataArray>(val)->getAsString());

    llvm::Function *printfFunc = module->getFunction("printf");
    std::vector<llvm::Value *> printfArgs;

    printfArgs.push_back(format_str);

    for (int i = 1; i < this->args.size(); ++i) {

      if (this->args.at(i)->get_type() == AST_NODE_EOF)
        break;

      llvm::Value *arg_val =
          this->args.at(i)->codegen(builder, context, module, variables);

      bool is_pointer_to_i8 =
          arg_val->getType()->isPointerTy() &&
          arg_val->getType()->getPointerElementType()->isIntegerTy(8);

      bool is_array_of_i8 =
          arg_val->getType()->isArrayTy() &&
          arg_val->getType()->getArrayElementType()->isIntegerTy(8);

      if ((is_pointer_to_i8 || is_array_of_i8) &&
          this->args.at(i)->get_type() != AST_NODE_VARIABLE_REFERENCE) {

        llvm::Value *arg_str = builder->CreateGlobalStringPtr(
            cast<llvm::ConstantDataArray>(arg_val)->getAsString());
        printfArgs.push_back(arg_str);
      } else {
        printfArgs.push_back(arg_val);
      }
    }

    return builder->CreateCall(printfFunc, printfArgs);
  } else {

    llvm::Function *func = module->getFunction(this->name);
    std::vector<llvm::Value *> funcArgs;

    for (int i = 0; i < this->args.size(); ++i) {
      if (this->args.at(i)->get_type() == AST_NODE_EOF)
        break;

      llvm::Value *arg_val =
          this->args.at(i)->codegen(builder, context, module, variables);

      bool is_pointer_to_i8 =
          arg_val->getType()->isPointerTy() &&
          arg_val->getType()->getPointerElementType()->isIntegerTy(8);

      bool is_array_of_i8 =
          arg_val->getType()->isArrayTy() &&
          arg_val->getType()->getArrayElementType()->isIntegerTy(8);

      if ((is_pointer_to_i8 || is_array_of_i8) &&
          this->args.at(i)->get_type() != AST_NODE_VARIABLE_REFERENCE) {

        llvm::Value *arg_str = builder->CreateGlobalStringPtr(
            cast<llvm::ConstantDataArray>(arg_val)->getAsString());
        funcArgs.push_back(arg_str);
      } else {
        funcArgs.push_back(arg_val);
      }
    }

    return builder->CreateCall(func, funcArgs);
  }

  return nullptr;
}

/* AST_BinaryOperation */

void AST_BinaryOperation::print(int indent) {
  printf("%sBinaryOperation(\n", boom_utils::indent_string(indent).c_str());
  printf("%s%s\n", boom_utils::indent_string(indent + 1).c_str(),
         this->op.c_str());
  this->left->print(indent + 1);
  this->right->print(indent + 1);
  printf("%s)\n", boom_utils::indent_string(indent).c_str());
}

llvm::Value *AST_BinaryOperation::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {

  auto lhs = this->left->codegen(builder, context, module, variables);

  if (this->op == "accessor") {
    llvm::Value *s_instance = lhs;

    if (this->right->get_type() != AST_NODE_VARIABLE_REFERENCE) {
      printf("Error, invalid accessor!");
      exit(1);
    }

    AST_VariableReference *accessor_ref = (AST_VariableReference *)this->right;

    // printf("\n----acc---\n");
    // s_instance->getType()->print(llvm::outs());
    // printf("\n----acc---\n");
    // std::cout << s_instance->getType()->getPointerElementType()->isStructTy()
    //           << std::endl;
    // printf("\n----acc---\n");

    std::string s_name = boom_utils::trim_string(
        s_instance->getType()->getPointerElementType()->getStructName().str());

    int accessor_index =
        (*variables)[s_name].struct_field_map[accessor_ref->name];

    std::cout << accessor_index << std::endl;

    std::vector<llvm::Value *> indices(2);
    indices[0] = builder->getInt32(0);
    indices[1] = builder->getInt32(accessor_index);

    llvm::PointerType *ptrType =
        static_cast<llvm::PointerType *>(s_instance->getType());
    llvm::Type *elementType = ptrType->getElementType();

    llvm::Value *fieldPtr =
        builder->CreateGEP(elementType, s_instance, indices, "fieldPtr");

    llvm::Value *fieldVal = builder->CreateLoad(
        fieldPtr->getType()->getPointerElementType(), fieldPtr);

    return fieldVal;
  }

  auto rhs = this->right->codegen(builder, context, module, variables);

  if (this->op == "=") {
    if (llvm::isa<llvm::LoadInst>(lhs)) {
      llvm::LoadInst *load_inst = (llvm::LoadInst *)lhs;
      llvm::Value *valPtr = load_inst->getPointerOperand();

      return builder->CreateStore(rhs, valPtr);
    } else {
      return builder->CreateStore(rhs, lhs);
    }
  }

  if (this->op == "+")
    return builder->CreateAdd(lhs, rhs);

  if (this->op == "-")
    return builder->CreateSub(lhs, rhs);

  if (this->op == "*")
    return builder->CreateMul(lhs, rhs);

  if (this->op == "/")
    return builder->CreateSDiv(lhs, rhs);

  if (this->op == "%")
    return builder->CreateSRem(lhs, rhs);

  if (this->op == ">")
    return builder->CreateICmpSGT(lhs, rhs);

  if (this->op == "<")
    return builder->CreateICmpSLT(lhs, rhs);

  if (this->op == "==")
    return builder->CreateICmpEQ(lhs, rhs);

  if (this->op == "index") {
    std::vector<llvm::Value *> indices(2);
    indices[0] = builder->getInt32(0);
    indices[1] = rhs;

    llvm::Value *elementPtr = builder->CreateInBoundsGEP(
        lhs->getType()->getPointerElementType(), lhs, indices, "elementPtr");

    llvm::LoadInst *elementValue = builder->CreateLoad(
        lhs->getType()->getPointerElementType()->getArrayElementType(),
        elementPtr);

    //     this->left->print();
    //     this->right->print();

    // printf("\n----ind-----\n");
    // elementValue->print(llvm::outs());
    // printf("\n----ind-----\n");

    return elementValue;
  }

  return nullptr;
}

/* AST_Conditional */

void AST_Conditional::print(int indent) {
  printf("%sConditional(\n", boom_utils::indent_string(indent).c_str());
  this->condition->print(indent + 1);
  this->onTrue->print(indent + 1);
  if (this->onFalse != nullptr) {
    this->onFalse->print(indent + 1);
  }
  printf("%s)\n", boom_utils::indent_string(indent).c_str());
}

llvm::Value *AST_Conditional::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {

  llvm::Value *Cond =
      this->condition->codegen(builder, context, module, variables);

  if (Cond == nullptr) {
    printf("Error generating conditional.\n");
    exit(1);
  }

  llvm::Function *TheFunction = builder->GetInsertBlock()->getParent();

  llvm::BasicBlock *OnTrueBB =
      llvm::BasicBlock::Create(*context, "then", TheFunction);
  llvm::BasicBlock *OnFalseBB = llvm::BasicBlock::Create(*context, "else");
  llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*context, "ifcont");

  builder->CreateCondBr(Cond, OnTrueBB, OnFalseBB);
  builder->SetInsertPoint(OnTrueBB);
  this->onTrue->codegen(builder, context, module, variables);

  bool onTrueHasReturn = does_block_end_in_return(this->onTrue);
  bool onFalseHasReturn = does_block_end_in_return(this->onFalse);

  // If the block has a return statement in it, dont create the branch.
  if (!onTrueHasReturn)
    builder->CreateBr(MergeBB);

  OnTrueBB = builder->GetInsertBlock();

  TheFunction->getBasicBlockList().push_back(OnFalseBB);
  builder->SetInsertPoint(OnFalseBB);
  this->onFalse->codegen(builder, context, module, variables);

  // If the block has a return statement in it, dont create the branch.
  if (!onFalseHasReturn)
    builder->CreateBr(MergeBB);

  OnFalseBB = builder->GetInsertBlock();

  if (!(onTrueHasReturn && onFalseHasReturn)) {
    TheFunction->getBasicBlockList().push_back(MergeBB);
  }

  builder->SetInsertPoint(MergeBB);

  return nullptr;
}

bool does_block_end_in_return(AST_Block *block) {
  if (block->nodes.size() < 2)
    return false;

  auto last_non_eof_node = block->nodes.at(block->nodes.size() - 2);

  if (last_non_eof_node->get_type() != AST_FUNCTION_CALL)
    return false;

  AST_FunctionCall *f_call = ((AST_FunctionCall *)last_non_eof_node);

  return f_call->name == "return";
}

/* AST_Loop */

void AST_Loop::print(int indent) {
  printf("%sLoop(\n", boom_utils::indent_string(indent).c_str());
  this->condition->print(indent + 1);
  this->expression->print(indent + 1);
  printf("%s)\n", boom_utils::indent_string(indent).c_str());
}

llvm::Value *AST_Loop::codegen(
    std::unique_ptr<llvm::IRBuilder<>> &builder,
    std::unique_ptr<llvm::LLVMContext> &context,
    std::unique_ptr<llvm::Module> &module,
    std::unique_ptr<std::map<std::string, VariableDefinition>> &variables) {

  llvm::Function *TheFunction = builder->GetInsertBlock()->getParent();

  llvm::BasicBlock *CondBB =
      llvm::BasicBlock::Create(*context, "whilecond", TheFunction);
  llvm::BasicBlock *LoopBB =
      llvm::BasicBlock::Create(*context, "loopbody", TheFunction);
  llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(*context, "loopend");

  builder->CreateBr(CondBB);
  builder->SetInsertPoint(CondBB);

  llvm::Value *Cond =
      this->condition->codegen(builder, context, module, variables);

  builder->CreateCondBr(Cond, LoopBB, EndBB);

  builder->SetInsertPoint(LoopBB);
  this->expression->codegen(builder, context, module, variables);
  builder->CreateBr(CondBB);

  TheFunction->getBasicBlockList().push_back(EndBB);
  builder->SetInsertPoint(EndBB);

  return nullptr;
}
