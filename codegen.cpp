// switching to llvm14

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

bool Codegen::all_branches_return(ASTNode * node){
    if (node->m_type == Jump_Statement && node->m_value == "RETURN"){
        return true;
    }
    else if (node->m_type == Block){
        for (int i = 0; i < node->m_children.size(); i++){
            if (all_branches_return(node->m_children[i])){
                return true;
            }
        }
        return false;
    }
    else if (node->m_type == Selection_Statement && node->m_value == "IF ELSE"){
        return all_branches_return(node->m_children[1]) && all_branches_return(node->m_children[2]);
    }

    else if (node->m_type == Selection_Statement && node->m_value == "IF"){
        return all_branches_return(node->m_children[1]);
    }
}

LLVMValueRef Codegen::declare_variable (std::string name, LLVMTypeRef type, bool local = true){
    std::cout << "declaring variable " << name << std::endl;

    if (!func_args.empty() && get_func_arg(name)){
        std::cout << "DOUBLE DECLARATION" << std::endl;
    } 


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

LLVMValueRef Codegen::get_func_arg(const std::string var_name) {
    std::map<std::string, LLVMValueRef> args = func_args.top();

    if (args.find(var_name) != args.end()){
        return args[var_name];
    }

    return nullptr;
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

    std::cout << "called get_node_value with type argument " << LLVMPrintTypeToString(type) << std::endl;

    if (node->m_type != Identifier) {
        return mem;
    } 

    std::string id = node->m_value;
    std::string load = "ld_" + id;
    // if (type == nullptr) std::cout << "nulltype" << std::endl;
    LLVMValueRef ld = LLVMBuildLoad(builderStack.top(), mem, load.c_str());
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

        std::map<std::string, LLVMValueRef> syms;

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
            function_types[node->m_children[1]->m_children[0]->m_value] = func_type;
        }

        else {
            // there are arguments, to be filled later
            std::vector<LLVMTypeRef> func_params;
            for(int i = 0; i < nargs; i++){
                ASTNode* arg = node->m_children[1]->m_children[1]->m_children[i];
                func_params.push_back(getLLVMType(arg->m_children[0]->m_value, context));
            }

            LLVMTypeRef* param_types = func_params.data();
            LLVMTypeRef func_type = LLVMFunctionType(func_return_type, param_types, nargs, 0);
            func_decl = LLVMAddFunction(cmodule, func_name, func_type);
            funcs.top()[func_name] = func_decl;   
            function_types[node->m_children[1]->m_children[0]->m_value] = func_type;
        }

        LLVMBasicBlockRef func_entry = LLVMAppendBasicBlock(func_decl, "entry");
        LLVMPositionBuilderAtEnd(builder, func_entry);

        // add argument variables to symbol table, to be done for functions with arguments

        for(int i = 0; i < nargs; i++){
            ASTNode* arg = node->m_children[1]->m_children[1]->m_children[i];
            std::string id = arg->m_children[1]->m_value;
            LLVMValueRef p = LLVMBuildAlloca(builder, getLLVMType(arg->m_children[0]->m_value,context), id.c_str());
            LLVMBuildStore(builder,LLVMGetParam(func_decl,i),p);
            std::cout << "here" << std::endl;
            syms[id] = p;
        }

        push_scope();
        func_args.push(syms);

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

    else if (node->m_type == Function_Call){
        std::string func_name = node->m_children[0]->m_value;
        LLVMValueRef id = get_func(func_name);

        LLVMTypeRef func_type = function_types[func_name]; 
        int func_nparams = LLVMCountParamTypes(func_type);

        vector<LLVMTypeRef> param_types (func_nparams);
        LLVMTypeRef* param_types_p = param_types.data();
        LLVMGetParamTypes(func_type, param_types_p);


        if (id) {
            if (node->m_children.size() == 1) { 
                // no arguments to the function
                LLVMValueRef* args;
                std::string func_call = func_name + "()";
                return LLVMBuildCall2(builder, LLVMTypeOf(id), id, args, 0, func_call.c_str());
            }
            else {
                std::vector<LLVMValueRef> func_args;
                std::string func_call = func_name + "()";

                for(int i = 0; i < (node->m_children[1]->m_children).size(); i++) {
                    LLVMValueRef arg = generate_code(node->m_children[1]->m_children[i], local, return_type);
                    LLVMValueRef arg_value = get_node_value(node->m_children[1]->m_children[i], param_types_p[i], arg);
                    func_args.push_back(arg_value);
                }


                LLVMValueRef* args = func_args.data();
                int nargs = func_args.size();

                std::cout << "done till here" << std::endl;
                return LLVMBuildCall2(builder, function_types[func_name], id, args, nargs, func_call.c_str());
            }
        }
    }

    else if (node->m_type == Block){
        std::cout << "generating block with return type " << LLVMPrintTypeToString(return_type) << std::endl;
        for(int i = 0; i < node->m_children.size(); i++){
            generate_code(node->m_children[i], local, return_type);
        }
    }

    else if (node->m_type == Assignment_Expression && node->m_children[1]->m_value == "="){
        std::string var_name = node->m_children[0]->m_value;
        LLVMValueRef mem = generate_code(node->m_children[0], local, return_type);
        LLVMValueRef rhs = generate_code(node->m_children[2], local, return_type);
        rhs = get_node_value(node->m_children[2], LLVMTypeOf(rhs), rhs);
        return LLVMBuildStore(builder, rhs, mem);
    }

    else if (node->m_type == Identifier){
        std::string name = node->m_value;

        LLVMValueRef id = var_to_val(name);
        LLVMValueRef id_f = get_func_arg(name);

        if (id) {
            return id;
        }

        else if (id_f){
            return id_f;
        }


        else{
            std::cout << "NO DECLARATION" << std::endl;
            return nullptr;
        }
        

        // else check if function argument
    }

    else if (node->m_type == Jump_Statement && node->m_value == "RETURN"){
        std::cout << "returning from a function of return type " << LLVMPrintTypeToString(return_type) << std::endl;
        if(return_type == LLVMVoidType()) {
            return LLVMBuildRetVoid(builder);
        } 
        else {
            LLVMValueRef return_val = generate_code(node->m_children[0], true, return_type);
            if (return_val == nullptr) std::cout << "nullpointer returned" << std::endl;
            return_val = get_node_value(node->m_children[0], return_type, return_val);
            return LLVMBuildRet(builder, return_val);
        }
    }

    else if (node->m_type == Multiplicative_Expression){
        LLVMValueRef op1 = generate_code(node->m_children[0], local, return_type);
        LLVMValueRef op2 = generate_code(node->m_children[1], local, return_type);

        if (LLVMGetTypeKind(LLVMTypeOf(op1)) == LLVMFloatTypeKind && 
            LLVMGetTypeKind(LLVMTypeOf(op2)) == LLVMFloatTypeKind) {
                op1 = get_node_value(node->m_children[0], LLVMFloatType(), op1);
                op2 = get_node_value(node->m_children[1], LLVMFloatType(), op2);
                return LLVMBuildBinOp(builder, ((LLVMOpcode)(LLVMMul + 1)), op1, op2, "_flt_op");
        }
        else {                
                op1 = get_node_value(node->m_children[0], LLVMInt32Type(), op1);
                op2 = get_node_value(node->m_children[1], LLVMInt32Type(), op2);
                return LLVMBuildBinOp(builder, LLVMMul, op1, op2, "_int_op");
        }
    }

    else if (node->m_type == Additive_Expression && node->m_value == "+"){
        LLVMValueRef op1 = generate_code(node->m_children[0], local, return_type);
        LLVMValueRef op2 = generate_code(node->m_children[1], local, return_type);

        if (LLVMGetTypeKind(LLVMTypeOf(op1)) == LLVMFloatTypeKind && 
            LLVMGetTypeKind(LLVMTypeOf(op2)) == LLVMFloatTypeKind) {
                op1 = get_node_value(node->m_children[0], LLVMFloatType(), op1);
                op2 = get_node_value(node->m_children[1], LLVMFloatType(), op2);
                return LLVMBuildBinOp(builder, ((LLVMOpcode)(LLVMAdd + 1)), op1, op2, "_flt_op");
        }
        else {                
                op1 = get_node_value(node->m_children[0], LLVMInt32Type(), op1);
                op2 = get_node_value(node->m_children[1], LLVMInt32Type(), op2);
                return LLVMBuildBinOp(builder, LLVMAdd, op1, op2, "_int_op");
        }
    }

    else if (node->m_type == Additive_Expression && node->m_value == "-"){
        LLVMValueRef op1 = generate_code(node->m_children[0], local, return_type);
        LLVMValueRef op2 = generate_code(node->m_children[1], local, return_type);

        if (LLVMGetTypeKind(LLVMTypeOf(op1)) == LLVMFloatTypeKind && 
            LLVMGetTypeKind(LLVMTypeOf(op2)) == LLVMFloatTypeKind) {
                op1 = get_node_value(node->m_children[0], LLVMFloatType(), op1);
                op2 = get_node_value(node->m_children[1], LLVMFloatType(), op2);
                return LLVMBuildBinOp(builder, ((LLVMOpcode)(LLVMSub + 1)), op1, op2, "_flt_op");
        }
        else {                
                op1 = get_node_value(node->m_children[0], LLVMInt32Type(), op1);
                op2 = get_node_value(node->m_children[1], LLVMInt32Type(), op2);
                return LLVMBuildBinOp(builder, LLVMSub, op1, op2, "_int_op");
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

    else if (node->m_type == F_Constant){
        LLVMConstReal(LLVMFloatTypeInContext(contextStack.top()), stof(node->m_value));
    }

    else if (node->m_type == String){
        return LLVMBuildGlobalStringPtr(builder, node->m_value.c_str(), "const_string");
    }

    else if (node->m_type == Relational_Expression){
        
        LLVMIntPredicate int_op;
        LLVMRealPredicate float_op;
        if (node->m_value == ">") {
            int_op = LLVMIntSGT;
            float_op = LLVMRealOGT;
        }
        else if (node->m_value == "<") {
            std::cout << "generating code for < condition" << std::endl;
            int_op = LLVMIntSLT;
            float_op = LLVMRealOLT;
        }


        LLVMValueRef op1 = generate_code(node->m_children[0], local, return_type);
        LLVMValueRef op2 = generate_code(node->m_children[1], local, return_type);

        if (LLVMGetTypeKind(LLVMTypeOf(op1)) == LLVMFloatTypeKind &&
            LLVMGetTypeKind(LLVMTypeOf(op2)) == LLVMFloatTypeKind){
                op1 = get_node_value(node->m_children[0], LLVMFloatType(), op1);
                op2 = get_node_value(node->m_children[1], LLVMFloatType(), op2);
                return LLVMBuildFCmp(builder, float_op, op1, op2, "flt_cmp");
	        }
            else{
                op1 = get_node_value(node->m_children[0],LLVMFloatType(), op1);
                op2 = get_node_value(node->m_children[1], LLVMFloatType(), op2);
                return LLVMBuildICmp(builder, int_op, op1, op2, "int_cmp");
            }
    }

    
    else if (node->m_type == Selection_Statement && node->m_value == "IF"){
        LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
        LLVMValueRef func = LLVMGetBasicBlockParent(current_block);

        LLVMBasicBlockRef if_block = LLVMAppendBasicBlock(func, "if");
        LLVMBasicBlockRef endif_block;
        
        endif_block = LLVMAppendBasicBlock(func, "endif");
        
        LLVMPositionBuilderAtEnd(builder, if_block);
        push_scope();
        generate_code(node->m_children[1], local, return_type);

        LLVMBuildBr(builder, endif_block);
        

        pop_scope();

        LLVMPositionBuilderAtEnd(builder, current_block);
        LLVMValueRef if_cond = generate_code(node->m_children[0], true, return_type);


        LLVMBuildCondBr(builder, if_cond, if_block, endif_block);
        LLVMPositionBuilderAtEnd(builder,endif_block);

        return nullptr;
    }

    else if (node->m_type == Selection_Statement && node->m_value == "IF ELSE"){
        LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
        LLVMValueRef func = LLVMGetBasicBlockParent(current_block);

        LLVMBasicBlockRef if_block = LLVMAppendBasicBlock(func, "if");
        LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(func, "else");
        LLVMBasicBlockRef endif_block;
        if (!all_branches_return(node)){
            endif_block = LLVMAppendBasicBlock(func, "endif");
        }
        
        LLVMPositionBuilderAtEnd(builder, if_block);
        push_scope();
        generate_code(node->m_children[1], local, return_type);

        if (!all_branches_return(node)){
            LLVMBuildBr(builder, endif_block);
        }

        pop_scope();

        LLVMPositionBuilderAtEnd(builder, else_block);
        push_scope();
        generate_code(node->m_children[2], local, return_type);

        if (!all_branches_return(node)){
            LLVMBuildBr(builder, endif_block);
        }

        pop_scope();

        LLVMPositionBuilderAtEnd(builder, current_block);
        LLVMValueRef if_cond = generate_code(node->m_children[0], true, return_type);

        LLVMBuildCondBr(builder, if_cond, if_block, else_block);

        if(!all_branches_return(node)){
            LLVMPositionBuilderAtEnd(builder,endif_block);
        }

        return nullptr;
    }

    else if (node->m_type == Iteration_Statement && node->m_value == "WHILE"){
        
        LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
        LLVMValueRef func = LLVMGetBasicBlockParent(current_block);

        LLVMBasicBlockRef while_block = LLVMAppendBasicBlock(func, "while");
        LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(func, "cond");
        LLVMBasicBlockRef endwhile_block = LLVMAppendBasicBlock(func, "endwhile");

        LLVMBuildBr(builder, cond_block);
        LLVMPositionBuilderAtEnd(builder, while_block);

        push_scope();
        generate_code(node->m_children[1], local, return_type);
        LLVMBuildBr(builder, cond_block);
        pop_scope();

        LLVMPositionBuilderAtEnd(builder, cond_block);
        LLVMValueRef while_cond = generate_code(node->m_children[0], true, return_type);
        LLVMBuildCondBr(builder, while_cond, while_block, endwhile_block);
        LLVMPositionBuilderAtEnd(builder, endwhile_block);
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
