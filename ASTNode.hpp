#include <string> 
#include <vector> 
#include <iostream> 
#include <map> 

enum NodeType {
    Begin,
    External_Declaration,
    Function_Definition,
    Declaration_Specifiers,
    Storage_Class_Specifier,
    Type_Specifier, 
    Type_Qualifier, 
    Function_Specifier, 
    Alignment_Specifier,
    Block,
    Declaration,
    Jump_Statement,
    Iteration_Statement,
    Expression_Statement,
    Selection_Statement,
    Labeled_Statement,
    Identifier,
    Expression,
    Assignment_Expression,
    Conditional_Expression,
    Assignment_Operator,
    Boolean_Expression
};


// std::vector<std::string> nodeTypetoString{
//     "Begin",
//     "Function",
//     "Block",
//     "Declaration",
//     "Jump_Statement",
//     "Iteration_Statement",
//     "Expression_Statement",
//     "Selection_Statement",
//     "Labeled_Statement",
//     "Identifier",
//     "Expression",
//     "Assignment_Expression",
//     "Conditional_Expression",
//     "Assignment_Operator",
//     "Boolean_Expression"
// };


class ASTNode {
    public:
    NodeType m_type;
    std::string m_value;
    std::vector<ASTNode*> m_children;


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

    void print(){
        std::cout << "hello" << std::endl;
        std::cout << m_children.size() << std::endl;
        // std::cout << nodeTypetoString[m_type] << std::endl;
        for(auto x:m_children){
            x->print();
        }
    }
    
};


