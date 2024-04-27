#include "scope.hpp"

scope::scope() {}

scope::scope(std::vector<std::string> syms, std::vector<std::string> locs) {
    for(int i = 0; i < syms.size(); i++) {
        symbols.push_back(syms[i]);
    }
    for(int j = 0; j < locs.size(); j++) {
        symbols.push_back(locs[j]);
    }
}

void scope::print_symbols() {
    for (int i = 0; i < symbols.size(); i++) {
        std::cout << symbols[i] << std::endl;
    }
    std::cout << "------------" << std::endl;
    for (int i = 0; i < locals.size(); i++) {
        std::cout << locals[i] << std::endl;
    }
}

void scope::add_symbol(std::string id) {
    locals.push_back(id);
}

bool scope::check_decl(std::string id, bool top = false) {
    int n = locals.size();
    for (int i = 0; i < n; i++) {
        if (locals[i] == id) {
            return true;
        }
    }
    return false;
}

bool scope::check_scope(std::string id, bool top = false) {
    int n = symbols.size();
    for (int i = 0; i < n; i++) {
        if (symbols[i] == id) {
            return true;
        }
    }
    int m = locals.size();
    for (int i = 0; i < m; i++) {
        if (locals[i] == id) {
            return true;
        }
    }
    return false;
}

scopeStack::scopeStack() {
    scope startscope = scope();
    scopes.push(startscope);
    // bool debug = false;
}

void scopeStack::enter_scope() {
    scope topscope = scopes.top();
    std::vector<std::string> syms = topscope.symbols;
    std::vector<std::string> locs = topscope.locals;
    scope newscope = scope(syms,locs);
    scopes.push(newscope);
}

void scopeStack::exit_scope() {
    scopes.pop();
}

bool scopeStack::check_node(ASTNode* node) {
    /* Main Function for checking 
    Scoping rules of the language */

    // Logic for checking node based on type of node here
    if(node == nullptr) return true;
    NodeType type = node->m_type;

    ASTNode* iter;
    ASTNode* block;
    std::string functionName, var;
    bool check = false;

    switch (type)
    {
        case (NodeType::Begin) :
            for(auto child : node->m_children)
            {
                bool flag = check_node(child);
                if(!flag) return false;
            }
            return true;
        case (NodeType::External_Declaration):
            return check_node(node->m_children[0]);
        case (NodeType::Function_Definition):
            functionName = ((node->m_children[1])->m_children[0])->m_value;

            if(scopes.top().check_scope(functionName))
            {
                std::cout << "Function " << functionName << " already defined" << std::endl;
                return false;
            }

            scopes.top().add_symbol(functionName);
            enter_scope();
            if(node->m_children.size() == 3){
                check = check_node(node->m_children[2]);
            }
            else{
                for(auto decl : node->m_children[2]->m_children)
                {
                    check = check_node(decl);
                    if(!check) return false;
                }
                check = check_node(node->m_children[3]);
            }
            exit_scope();
            return check;
        case (NodeType::Declaration):
            for(auto decl : node->m_children[1]->m_children) // init declarator list
            {
                iter = decl->m_children[0]; 
                while(iter->m_children.size()==2) {
                    iter = iter->m_children[1];
                }
                var = iter->m_children[0]->m_value;

                if(scopes.top().check_decl(var))
                {
                    std::cout << "Variable " << var << " already defined" << std::endl;
                    return false;
                }
                scopes.top().add_symbol(decl->m_value);
            }
            return true;
        case (NodeType::Compound_Statement):
            block = node->m_children[0];
            if(block->m_value == "") return true;

            for(auto child : block->m_children)
            {
                check = check_node(child);
                if(!check) return false;
            }
            return true;
        case (NodeType::Selection_Statement):
            if(node->m_value == "IF")
            {
                check = check_node(node->m_children[0]);
                if(!check) return false;

                enter_scope();
                check = check_node(node->m_children[1]);
                if(!check) return false;
                exit_scope();
            }
            if(node->m_value == "IF ELSE")
            {
                check = check_node(node->m_children[0]);
                if(!check) return false;

                enter_scope();
                check = check_node(node->m_children[1]);
                if(!check) return false;
                exit_scope();

                enter_scope();
                check = check_node(node->m_children[2]);
                if(!check) return false;
                exit_scope();
            }
            return true;
        case (NodeType::Iteration_Statement):
            if(node->m_value == "WHILE")
            {
                check = check_node(node->m_children[0]);
                if(!check) return false;

                enter_scope();
                check = check_node(node->m_children[1]);
                if(!check) return false;
                exit_scope();
            }
            return true;
        case (NodeType::Jump_Statement):
            if((node->m_value == "RETURN") && (node->m_children.size() > 0))
            {
                return check_node(node->m_children[0]);
            }
            return true;
        case (NodeType::Expression):
        case (NodeType::Assignment_Expression):
        case (NodeType::Conditional_Expression):
        case (NodeType::Logical_Or_Expression):
        case (NodeType::Logical_And_Expression):
        case (NodeType::Inclusive_Or_Expression):
        case (NodeType::Exclusive_Or_Expression):
        case (NodeType::And_Expression):
        case (NodeType::Equality_Expression):
        case (NodeType::Relational_Expression):
        case (NodeType::Shift_Expression):
        case (NodeType::Additive_Expression):
        case (NodeType::Multiplicative_Expression):
        case (NodeType::Unary_Expression):
        case (NodeType::Postfix_Expression):
        case (NodeType::Argument_Expression_List):
            for(auto child : node->m_children)
            {
                check = check_node(child);
                if(!check) return false;
            }
            return true;
        case (NodeType::Cast_Expression):
            return check_node(node->m_children[1]);
        case (NodeType::Identifier):
            return scopes.top().check_decl(node->m_value);
        default:
            return true;
    }
    return true;
}