#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <memory> 

#include "ASTNode.hpp"
#include "codegen.hpp"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <llvm-c/Core.h>

using namespace std;
using namespace llvm;

std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<Module> TheModule;
std::unique_ptr<IRBuilder<>> Builder;
std::map<std::string, Value *> NamedValues;


Codegen::Codegen() {
    LLVMContextRef globalContext = LLVMGetGlobalContext();
    LLVMBuilderRef globalBuilder = LLVMCreateBuilderInContext(globalContext);

    // initialize scope
    vars.push(std::map<std::string, LLVMValueRef>());
    func_args.push(std::map<std::string, LLVMValueRef>());
    funcs.push(std::map<std::string, LLVMValueRef>());

    contextStack.push(globalContext);
    builderStack.push(globalBuilder);
    
    cmodule = LLVMModuleCreateWithNameInContext("cmodule", globalContext);
}

void Codegen::push_scope(){
    vars.push(std::map<std::string, LLVMValueRef>());
}

void Codegen::pop_scope(){
    vars.pop();
}

LLVMValueRef Codegen::declare_variable (std::string name, LLVMTypeRef type, bool local = true){
    std::cout << "declaring variable " << name << std::endl;
    LLVMValueRef mem;
    LLVMBuilderRef builder = builderStack.top();

    if (local) {
        mem = LLVMBuildAlloca(builder, type, name.c_str());
    } 
    else {
        mem = LLVMAddGlobal(cmodule, type, name.c_str());
        LLVMSetLinkage(mem, LLVMCommonLinkage);
        // The symbol is not a global constant
        LLVMSetGlobalConstant(mem,0);
        // Initialize globals to 0 
        LLVMTypeKind tkind = LLVMGetTypeKind(type);
        LLVMValueRef z;
        if (tkind == LLVMIntegerTypeKind) {
            z = LLVMConstInt(LLVMInt32TypeInContext(contextStack.top()), 0, false);
        } else {
            z = LLVMConstReal(LLVMFloatTypeInContext(contextStack.top()), 0.0);
        }
        LLVMSetInitializer(mem, z);
    }
    vars.top()[name] = mem;
    return mem;
}

LLVMValueRef Codegen::var_to_val(std::string var_name) {
    if (vars.empty()) {
        return nullptr;
    } 

    // cannot use reference as we might have to search recursively, and hence need a copy
    std::map<std::string, LLVMValueRef> scope = vars.top();
    if (scope.find(var_name) != scope.end()) {
        return scope[var_name];
    } 
    else {
        vars.pop();
        LLVMValueRef var = var_to_val(var_name);
        vars.push(scope);
        return var;
    }
}

LLVMValueRef Codegen::get_func(std::string func_name) {
    if (funcs.empty()) {
        return nullptr;
    } 

    std::map<std::string, LLVMValueRef> scope = funcs.top();
    if (scope.find(func_name) != scope.end()) {
        return scope[func_name];
    } 
    else {
        funcs.pop();
        LLVMValueRef func = get_func(func_name);
        funcs.push(scope);
        return func;
    }
}

LLVMTypeRef getLLVMType(std::string type, LLVMContextRef context) {
    if (type == "INT") {
        return LLVMInt32TypeInContext(context);
    } 
    else if (type == "FLOAT") {
        return LLVMFloatTypeInContext(context);
    } 
    else if (type == "VOID") {
        return LLVMVoidTypeInContext(context);
    } 
    else if (type == "CHAR") {
        return LLVMInt8TypeInContext(context);
    } 

    // else {
    //     std::string cmp; 
    //     cmp += type_qualifier.back();
    //     if (cmp == "*") { 
    //         type_qualifier.pop_back();
    //         LLVMTypeRef base_type = getLLVMType(type_qualifier, context);
    //         return LLVMPointerType(base_type,0);
    //     } else {
    //         return LLVMInt32TypeInContext(context);
    //     }
    //     // return NULL;
    // }

    return nullptr;
}





LLVMValueRef Codegen::get_node_value(ASTNode* node, LLVMTypeRef type, LLVMValueRef mem) {
    // std::cout << type << std::endl;

    if (node->m_type != Identifier) {
        return mem;
    } 

    std::string id = node->m_value;
    std::string load = "ld_" + id;
    // if (type == nullptr) std::cout << "nulltype" << std::endl;
    LLVMValueRef ld = LLVMBuildLoad2(builderStack.top(), type, mem, load.c_str());
    // LLVMValueRef ld = LLVMBuildLoad(builderStack.top(), mem, load.c_str());
    std::cout << "getting node value" << std::endl;
    return ld;
}






LLVMValueRef Codegen::generate_code(ASTNode* node, bool local = true, LLVMTypeRef return_type = nullptr){
    LLVMContextRef context = contextStack.top();
    LLVMBuilderRef builder = builderStack.top();

    if (node->m_type == Begin){
        std::cout << "Generating code for Begin" << std::endl;
        for(auto x : node->m_children){
            generate_code(x, local, return_type);
        }
        return nullptr;
    }


    else if (node->m_type == Function_Definition){
        std::string return_type_str = node->m_children[0]->m_value;
        std::cout << "return type of function is " << return_type_str << std::endl;
        LLVMTypeRef func_return_type = getLLVMType(return_type_str, context);
        const char* func_name = node->m_children[1]->m_children[0]->m_value.c_str();
        std::cout << "function name is " << func_name << std::endl;

        LLVMValueRef func_decl;
        std::map<std::string, LLVMValueRef> ftable = funcs.top();

        // if function is already declared
        if (ftable.find(func_name) != ftable.end()) {
            return ftable[func_name];
        }

        int nargs = 0;
        if ((node->m_children[1]->m_children).size() > 1 && node->m_children[1]->m_children[1]->m_type == Parameter_List){
            nargs = (node->m_children[1]->m_children[1]->m_children).size();
        }

        std::cout << "number of arguments: " << nargs << std::endl;


        if (nargs == 0) {
            LLVMTypeRef func_type = LLVMFunctionType(func_return_type, {}, 0, 0);
            func_decl = LLVMAddFunction(cmodule, func_name, func_type);
            unsigned int ct = LLVMCountParams(func_decl);
            funcs.top()[func_name] = func_decl;
        }

        else {
            // there are arguments, to be filled later
        }

        LLVMBasicBlockRef func_entry = LLVMAppendBasicBlock(func_decl, "entry");
        std::cout << "here" << std::endl;
        LLVMPositionBuilderAtEnd(builder, func_entry);

        // add argument variables to symbol table, to be done for functions with arguments

        push_scope();
        func_args.push(std::map<std::string, LLVMValueRef>());

        // generate code for code block
        // generate_code(node->m_children[2]);
        generate_code(node->m_children[2], local, func_return_type);

        pop_scope();
        func_args.pop();


        if (return_type_str == "VOID"){
            LLVMBuildRetVoid(builder);
        }

        return nullptr;

    }

    else if (node->m_type == Block){
        for(int i = 0; i < node->m_children.size(); i++){
            generate_code(node->m_children[i], local, return_type);
        }
    }

    else if (node->m_type == Identifier){
        std::string name = node->m_value;
        
        LLVMValueRef id = var_to_val(name);
        if (id) {
            return id;
        }

        // else check if function argument
    }

    else if (node->m_type == Jump_Statement && node->m_value == "RETURN"){
        if(return_type == LLVMVoidType()) {
            return LLVMBuildRetVoid(builder);
        } 
        else {
            LLVMValueRef return_val = generate_code(node->m_children[0], true, return_type);\
            if (return_val == nullptr) std::cout << "nullpointer returned" << std::endl;
            return_val = get_node_value(node->m_children[0], return_type, return_val);
            return LLVMBuildRet(builder, return_val);
        }
    }






    else if (node->m_type == Declaration){
        LLVMTypeRef var_type = getLLVMType(node->m_children[0]->m_value, context);

        int ndecl = (node->m_children[1]->m_children).size();
        std::cout << "declared " << ndecl << " variables of type " << node->m_children[0]->m_value << std::endl;
        
        for(int i = 0; i < ndecl; i++){
            declare_variable(node->m_children[1]->m_children[i]->m_children[0]->m_value, var_type);
        }
    }


    else if (node->m_type == I_Constant){
        std::cout << "processing constant " << node->m_value << std::endl;
        return LLVMConstInt(LLVMInt32TypeInContext(context), stoi(node->m_value), false);
    }


    else{
        for(auto x : node->m_children){
            generate_code(x, local, return_type);
        }
        return nullptr; 
    }

}


// std::string get_function_return_type(ASTNode* node){
//     std::cout << "getting return type for function" << std::endl;
//     if (node->m_rule == 3){
//         ASTNode* type_specifier = node->m_children[0];
//         if (type_specifier->m_value != ""){
//             return type_specifier->m_value;
//         }
//     }
//     else if (node->m_rule == 4){
//         ASTNode* type_specifier = node->m_children[0];
//         if (type_specifier->m_value != ""){
//             return type_specifier->m_value;
//         }
//     }
//     else if (node->m_rule % 2 == 1){
//         return get_function_return_type(node->m_children[1]);
//     }
// }

// std::string get_function_name();





// void codegen(ASTNode* node){
//     std::cout << "calling codegen on node" << std::endl;

//     // if (node->m_type == Begin){
//     //     if (node->m_rule == 1){
//     //         codegen(node->m_children[0]);
//     //     }
//     //     else if (node->m_rule == 2){
//     //         codegen(node->m_children[0]);
//     //         codegen(node->m_children[1]);
//     //     }
//     // }

//     // else if (node->m_type == External_Declaration){
//     //     if (node->m_rule == 1){
//     //         codegen(node->m_children[]);
//     //     }
//     // }

//     if (node->m_type == Function_Definition) {
//         // get the return type of the function from the declaration specifier
//         std::string return_type_string = get_function_return_type(node->m_children[0]);  
//         cout << return_type_string << endl;

//     }

//     else {
//         for(auto child : node->m_children){
//             codegen(child);
//         }
//     }
// }


void Codegen::convert_to_ir(ASTNode* node, std::string file) {
    generate_code(node);

    std::ofstream outfile;
    std::string filename = file;
    outfile.open(filename);
    outfile << LLVMPrintModuleToString(cmodule) << std::endl;
    outfile.close();
}
