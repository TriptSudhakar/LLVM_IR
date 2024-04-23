#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ASTNode.hpp"
#include "c.tab.hpp"
#include "codegen.cpp"
#include "scope.cpp"


#include <vector> 


extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;

static void usage()
{
  printf("Usage: cc <prog.c>\n");
}


std::vector<std::string> nodeTypetoString{
    "Begin",
    "External_Declaration",
    "Function_Definition",
    "Declaration_Specifiers",
    "Declaration",
    "Declaration_List",
    "Storage_Class_Specifier",
    "Type_Specifier", 
    "Declarator",
    "Direct_Declarator",
    "Compound_Statement",
    "Statement",
    "Type_Qualifier", 
    "Function_Specifier", 
    "Alignment_Specifier",
    "Block",
    "Jump_Statement",
    "Iteration_Statement",
    "Expression_Statement",
    "Selection_Statement",
    "Labeled_Statement",
    "Expression",
    "Assignment_Expression",
    "Conditional_Expression",
    "Assignment_Operator",
    "Logical_Or_Expression",
    "Logical_And_Expression",
    "Inclusive_Or_Expression",
    "Exclusive_Or_Expression",
    "And_Expression",
    "Equality_Expression",
    "Relational_Expression",
    "Shift_Expression",
    "Additive_Expression",
    "Multiplicative_Expression",
    "Cast_Expression",
    "Unary_Expression", 
    "Postfix_Expression",
    "Unary_Operator",
    "Primary_Expression",
    "Identifier_List",
    "Identifier",
    "Argument_Expression_List",
    "Constant",
    "I_Constant",
    "F_Constant",
    "String",
    "Parameter_Type_List",
    "Parameter_List",
    "Parameter_Declaration",
    "Abstract_Declarator",
    "Type_Qualifier_List",
    "Pointer",
    "Init_Declarator_List", 
    "Init_Declarator",
    "Function_Call"
};


void printHelper(ASTNode* node, int n, std::vector<int>& formattingVector){
    std::string formatter = "";

    while (formattingVector.size() <= n){
      // formattingVector.push_back((node->m_children).size());
      formattingVector.push_back(0);
    }

    formattingVector[n] = (node->m_children).size(); 


    for(int i = 0; i < n; i++){

        if (formattingVector[i] > 0){
          formatter.push_back('|');
        }
        else{
          formatter.push_back(' ');
        }


        if (i == n - 1){
          formatter.push_back('-');
        }
        else{
          formatter.push_back(' ');
        }
    }

    std::cout << formatter;
    std::cout << nodeTypetoString[node->m_type];
    if (node->m_value != "") std::cout << "    " << "(" << node->m_value << ")";
    std::cout << std::endl;


    if(n>1) formattingVector[n-1] -= 1;


    for(auto x:(node -> m_children)){
        printHelper(x, n+1, formattingVector);
    }
}


void print(ASTNode* node){
    std::vector<int> formattingVector;
    printHelper(node, 0, formattingVector);
}


int
main(int argc, char **argv)
{
  if (argc != 2) {
    usage();
    exit(1);
  }
  char const *filename = argv[1];
  yyin = fopen(filename, "r");
  assert(yyin);
  extern ASTNode* root;
  int ret = yyparse();

  print(root);
  // root->print();

  scopeStack scopes = scopeStack();
    bool scopechk = scopes.check_node(root);
    if (!scopechk) {
      std::cout << "Scoping check failed." << std::endl;
      ret = 1;
      printf("retv = %d\n", ret);
      exit(1);
    }
    
  printf("retv = %d\n", ret);

  // codegen();
  Codegen cgen = Codegen();
  cgen.convert_to_ir(root, "ir_dump.ll");
  exit(0);
}