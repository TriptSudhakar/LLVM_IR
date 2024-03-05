#include <string> 
#include <vector> 
#include <iostream> 

enum NodeType {
    Begin
};


class ASTNode {
    public:
    NodeType m_type;
    std::string m_value;
    std::vector<ASTNode*> m_children;


    // ASTNode() {};
    
    // ASTNode(NodeType type) : 
    //     m_type (type) {};

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
    }
    
};


