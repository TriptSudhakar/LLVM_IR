#include "ASTNode.hpp"
#include "llvm.hpp"
#include <stack>
#include <vector>
#include <map>    
#include <iostream>
#include <string>
#include <memory> 
#include <fstream> 


class Codegen{
public:
        LLVMModuleRef cmodule;

        // symbol tables
        std::stack<std::map<std::string, LLVMValueRef>> vars;
        std::stack<std::map<std::string, LLVMValueRef>> funcs;
        std::stack<std::map<std::string, LLVMValueRef>> func_args;

        std::stack<LLVMContextRef> contextStack;
        std::stack<LLVMBuilderRef> builderStack;
        std::vector<LLVMBasicBlockRef> labels;
       
        // constructor method
        Codegen(); 

        void push_scope();
        void pop_scope();

        LLVMValueRef var_to_val(std::string var_name);
        LLVMValueRef get_func(std::string func_name);

        LLVMValueRef generate_code(ASTNode* node);

        void convert_to_ir(ASTNode* node, std::string file);
};