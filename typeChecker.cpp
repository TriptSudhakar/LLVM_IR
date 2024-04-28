#include "typeChecker.hpp"


typeChecker::typeChecker() {
    getType[0] = "INT";
    getType[1] = "FLOAT";
    getType[2] = "VOID";
    getType[3] = "CHAR*";

    getTypeId["INT"]    = 0;
    getTypeId["CHAR"]   = 0;
    getTypeId["FLOAT"]  = 1;
    getTypeId["VOID"]   = 2;
    getTypeId["CHAR*"]  = 3;

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
    if (node == NULL) return 2;
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

            if(lookup_sym(functionName) != -1) return -1;
            add_symbol(functionName, functionType);
            fargs[functionName] = std::vector<int>();
            isVariadic[functionName] = false;

            enter_scope();
            int check = 0;
            if(node->m_children[1]->m_children.size() > 1) {
                if((node->m_children[1])->m_children[1]->m_value == "..." ) isVariadic[functionName] = true;
                for(auto child : ((node->m_children[1])->m_children[1])->m_children) {
                    std::string argType = child->m_children[0]->m_value;
                    std::string argName;
                    if(child->m_children[1]->m_children.size() > 0)
                    { 
                        argType.append("*");
                        argName = child->m_children[1]->m_children[1]->m_value;
                    }
                    else argName = child->m_children[1]->m_value;

                    add_symbol(argName, argType);
                    fargs[functionName].push_back(getTypeId[argType]);
                }
            }

            check = check_node(node->m_children[2]);
            exit_scope();

            if(check < 0) return -1;
            std::cout<<"Function " << functionName << " returns " << functionType << std::endl;
            return getTypeId[functionType];
        }
        case (NodeType::Function_Call):
        {
            std::string functionName = node->m_children[0]->m_value;
            if(fargs.find(functionName) == fargs.end()) return -1;

            int argc = 0;
            for(auto child : node->m_children[1]->m_children) {
                int check = check_node(child);
                if(check < 0) return -1;

                if(isVariadic[functionName]) {
                    if(argc < fargs[functionName].size() && check != fargs[functionName][argc]) return -1;
                }
                else if(argc >= fargs[functionName].size()) return -1;
                else if(check != fargs[functionName][argc]) return -1;
                argc++;
            }
            return lookup_sym(functionName);
        }
        case (NodeType::Declaration):
        {
            std::string variableType = (node->m_children[0])->m_value;
            for(auto child : node->m_children[1]->m_children) {
                if(child->m_children[0]->m_children.size() > 0)
                {
                    std::string functionName = ((child->m_children[0])->m_children[0])->m_value;

                    if(lookup_sym(functionName) != -1) return -1;
                    add_symbol(functionName, variableType);
                    fargs[functionName] = std::vector<int>();
                    isVariadic[functionName] = false;

                    int check = 0;
                    if((child->m_children[0])->m_children[1]->m_value == "..." ) isVariadic[functionName] = true;

                    for(auto decl : ((child->m_children[0])->m_children[1])->m_children) {
                        std::string argType = decl->m_children[0]->m_value;
                        if(decl->m_children[1]->m_children.size() > 0) argType.append("*");
                        fargs[functionName].push_back(getTypeId[argType]);
                    }

                    std::cout<<"Function " << functionName << " returns " << variableType << std::endl;
                }
                else
                {
                    std::string id = child->m_children[0]->m_value;
                    add_symbol(id, variableType);

                    if(child->m_children.size() > 1) {
                        int check = check_node(child->m_children[1]);
                        if(check < 0 || check != getTypeId[variableType]) return -1;
                    }
                }
            }
            return getTypeId[variableType];
        }
        case (NodeType::Block):
        {
            int check = 2;
            for(auto child : node->m_children) {
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
            return 2;
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
            if(node->m_value == "DO WHILE") {
                enter_scope();
                check = check_node(node->m_children[0]);
                if(check < 0) return -1;
                exit_scope();

                check = check_node(node->m_children[1]);
                if(check < 0) return -1;
            }
            if(node->m_value == "FOR") {
                for(int i = 0; i < node->m_children.size()-1; i++)
                {
                    check = check_node(node->m_children[i]);
                    if(check < 0) return -1;
                }

                enter_scope();
                check = check_node(node->m_children[node->m_children.size()-1]);
                if(check < 0) return -1;
                exit_scope();
            }
            return 2;
        }
        case (NodeType::Jump_Statement):
        {
            if(node->m_value == "RETURN") {
                return check_node(node->m_children[0]);
            }
            return 2;
        }
        case (NodeType::Expression):
        {
            int check = 0;
            for(auto child : node->m_children) {
                check = check_node(child);
                if(check < 0) return -1;
            }
            return check;
        }
        case (NodeType::Assignment_Expression):
        {
            int check = 0;
            check = check_node(node->m_children[2]);
            if(check < 0 || check != check_node(node->m_children[0])) return -1;
            if(check == 1)
            {
                std::string op = node->m_children[1]->m_value;
                if(op != "=" && op != "+=" && op != "-=" && op != "*=" && op != "/=") return -1;
            }
            return check;
        }
        case (NodeType::Conditional_Expression):
        {
            int check = 0;
            int left = 0;
            int right = 0;
            check = check_node(node->m_children[0]);
            if(check != 0) return -1;
            left = check_node(node->m_children[1]);
            if(left < 0) return -1;
            right = check_node(node->m_children[2]);
            if(right < 0) return -1;

            if(left != right) return -1;
            check = left;
            return check;
        }
        case (NodeType::Inclusive_Or_Expression):
        case (NodeType::Exclusive_Or_Expression):
        case (NodeType::And_Expression):
        {
            int check = 0;
            for(auto child : node->m_children) {
                check = check_node(child);
                if(check != 0) return -1;
            }
            return check;
        }
        case (NodeType::Logical_Or_Expression):
        case (NodeType::Logical_And_Expression):
        case (NodeType::Equality_Expression):
        case (NodeType::Relational_Expression):
        {
            int check = 0;
            for(auto child : node->m_children) {
                check = check_node(child);
                if(check < 0) return -1;
            }
            return 0;
        }
        case (NodeType::Shift_Expression):
        {
            int check = 0;
            for(auto child : node->m_children) {
                check = check_node(child);
                if(check != 0) return -1;
            }
            return check;
        }
        case (NodeType::Additive_Expression):
        case (NodeType::Multiplicative_Expression):
        {
            int ans = 0;
            int check = 0;
            for(auto child : node->m_children) {
                check = check_node(child);
                if(check < 0) return -1;
                ans = std::max(ans, check);
            }
            return ans;
        }
        case (NodeType::Unary_Expression):
        {
            int check = 0;
            check = check_node(node->m_children.back());
            if(check < 0) return -1;
            return check;
        }
        case (NodeType::Postfix_Expression):
        {
            int check = 0;
            check = check_node(node->m_children[0]);
            if(check < 0) return -1;
            return check;
        }
        case (NodeType::Identifier):
        {
            std::string var = node->m_value;
            return symbols.top()[var];
        }
        case (NodeType::I_Constant): return 0;
        case (NodeType::F_Constant): return 1;
        case (NodeType::String): return 3;
    }
    return 0;
}