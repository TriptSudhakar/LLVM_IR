#ifndef _AST_NODE_HH_
#define _AST_NODE_HH_

#include <string> 
#include <vector> 
#include <iostream> 
#include <map> 

enum NodeType {
    Begin,
    External_Declaration,
    Function_Definition,
    Declaration_Specifiers,
    Declaration,
    Storage_Class_Specifier,
    Type_Specifier, 
    Declarator,
    Direct_Declarator,
    Compound_Statement,
    Statement,
    Type_Qualifier, 
    Function_Specifier, 
    Alignment_Specifier,
    Block,
    Jump_Statement,
    Iteration_Statement,
    Expression_Statement,
    Selection_Statement,
    Labeled_Statement,
    Expression,
    Assignment_Expression,
    Conditional_Expression,
    Assignment_Operator,
    Logical_Or_Expression,
    Logical_And_Expression,
    Inclusive_Or_Expression,
    Exclusive_Or_Expression,
    And_Expression,
    Equality_Expression,
    Relational_Expression,
    Shift_Expression,
    Additive_Expression,
    Multiplicative_Expression,
    Cast_Expression,
    Unary_Expression, 
    Postfix_Expression,
    Unary_Operator,
    Primary_Expression,
    Identifier_List,
    Identifier,
    Argument_Expression_List,
    Constant,
    I_Constant,
    F_Constant,
    String,
    Parameter_Type_List,
    Parameter_List,
    Parameter_Declaration,
    Abstract_Declarator,
    Type_Qualifier_List,
    Pointer,
    Init_Declarator_List, 
    Init_Declarator
};





class ASTNode {
    public:
    NodeType m_type;
    std::string m_value;
    std::vector<ASTNode*> m_children;
    int m_rule;


    ASTNode() {};
    
    ASTNode(NodeType type) : 
        m_type (type) {};
    
    // ASTNode(NodeType type, std::string& value, std::vector<ASTNode*> children) : 
    //     m_type (type), 
    //     m_value (value), 
    //     m_children (children) {};

    void pushChild(ASTNode* node){
        m_children.push_back(node);
    }

    void pushChildren(std::vector<ASTNode*>& children){
        for(std::vector<ASTNode*>::iterator child = children.begin(); child != children.end(); child++){
            this->pushChild(*child);
        }
    }

    void printHelper(int n){
        std::string formatter = "";
        for(int i = 0; i < n; i++){
            formatter.push_back(' ');
            formatter.push_back(' ');
        }
        std::cout << formatter;
        std::cout << m_type;
        if (m_value != "") std::cout << formatter << m_value << std::endl;
        for(auto x:m_children){
            x->printHelper(n+1);
        }
    }

    void print(){
        printHelper(0);
    }
};

// class ASTNodeVector{
// public:
//     std::vector<ASTNode*> node_vector;

//     void add_node(ASTNode* node){
//         node_vector.push_back(node);
//     }

//     ASTNode* get_node(int n){
//         return node_vector[n];
//     }

//     int get_length(){
//         return node_vector.size();
//     }
// };

#endif