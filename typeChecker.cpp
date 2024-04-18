#include "typeChecker.hpp"


typeChecker::typeChecker() {
    getType[0] = "INT";
    // getType[1] = "FLOAT";
    // getType[2] = "CHAR*";
    // getType[3] = "INTARR";
    // getType[4] = "FLOATARR";
    getType[5] = "VOID";

    getTypeId["INT"]    = 0;
    // getTypeId["FLOAT"]  = 1;
    // getTypeId["CHAR*"]  = 2;
    // getTypeId["INTARR"]   = 3;
    // getTypeId["FLOATARR"]   = 4;
    getTypeId["VOID"]   = 5;

    std::map<std::string,int> initmap;
    symbols.push(initmap);
}

void typeChecker::enter_scope() {
    std::map<std::string,int> topmap = symbols.top();
    symbols.push(topmap); 
}

void typeChecker::exit_scope() {
    symbols.pop();
}

void typeChecker::add_symbol(std::string id, std::string idtype) {
    symbols.top()[id] = getTypeId[idtype];
}

int typeChecker::lookup_sym(std::string id){
    std::map<std::string,int> topmap = symbols.top();
    if (topmap.find(id) != topmap.end()) {
        return topmap[id];
    }
    return -1;
}

int typeChecker::check_node(ASTNode* node) {
    if (node == NULL) return 5;
    NodeType type = node->m_type;

    switch (type) {
        case (NodeType::Begin):
        {
            int check = 0;
            for(auto child : node->m_children) {
                check = check_node(child);
                if(check < 0) return -1;
            }
            return check;
        }
        case (NodeType::External_Declaration):
            return check_node(node->m_children[0]);
        case (NodeType::Function_Definition):
        {
            std::string functionName = ((node->m_children[1])->m_children[0])->m_value;
            std::string functionType = (node->m_children[0])->m_value;

            add_symbol(functionName, functionType);
            fargs[functionName] = std::vector<int>();

            enter_scope();
            int check = 0;
            if(node->m_children.size() > 2) {
                for(auto child : node->m_children[2]->m_children) {
                    check = check_node(child);
                    if(check < 0) return -1;
                }

                check = check_node(node->m_children[3]);
                if(check < 0) return -1;
            }
            else
            {
                for(auto child : node->m_children[2]->m_children) {
                    check = check_node(child);
                    if(check < 0) return -1;
                }
            }
            exit_scope();
            return getTypeId[functionType];
        }
        case (NodeType::Declaration):
        {
            std::string variableType = (node->m_children[0])->m_value;
            for(auto child : node->m_children[1]->m_children) {
                std::string id = ((child->m_children[0])->m_children[0])->m_value;
                add_symbol(id, variableType);
            }
            return getTypeId[variableType];
        }
        case (NodeType::Compound_Statement):
        {
            ASTNode* block = node->m_children[0];
            int check = 0;
            if(block->m_value == "") return 5;

            for(auto child : block->m_children) {
                check = check_node(child);
                if(check < 0) return -1;
            }
            return check;
        }
        case (NodeType::Selection_Statement):
        {
            int check = 0;
            if(node->m_value == "IF") {
                check = check_node(node->m_children[0]);
                if(check < 0) return -1;

                enter_scope();
                check = check_node(node->m_children[1]);
                if(check < 0) return -1;
                exit_scope();
            }
            if(node->m_value == "IF ELSE") {
                check = check_node(node->m_children[0]);
                if(check < 0) return -1;

                enter_scope();
                check = check_node(node->m_children[1]);
                if(check < 0) return -1;
                exit_scope();
                
                enter_scope();
                check = check_node(node->m_children[2]);
                if(check < 0) return -1;
                exit_scope();
            }
            return 5;
        }
        case (NodeType::Iteration_Statement):
        {
            int check = 0;
            if(node->m_value == "WHILE") {
                check = check_node(node->m_children[0]);
                if(check < 0) return -1;
                enter_scope();
                check = check_node(node->m_children[1]);
                if(check < 0) return -1;
                exit_scope();
            }
            return 5;
        }
        case (NodeType::Jump_Statement):
        {
            if(node->m_value == "RETURN") {
                return check_node(node->m_children[0]);
            }
            return 5;
        }
        case (NodeType::Expression):
        {
            
        }
    }
    return 0;
}
