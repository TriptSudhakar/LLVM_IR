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
    NodeType type = node->m_type;

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
            std::string functionName;
            ASTNode* iter = node->m_children[1]; // declarator
            while(iter->m_children.size()==2) {
                iter = iter->m_children[1];
            }
            functionName = iter->m_children[0]->m_value;

            if(scopes.top().check_scope(functionName))
            {
                std::cout << "Function " << functionName << " already defined" << std::endl;
                return false;
            }

            scopes.top().add_symbol(functionName);
            enter_scope();
            bool check = check_node(node->m_children[2]);
            exit_scope();
            return check;
        case (NodeType::Declaration):
            for(auto decl : node->m_children[1]->m_children) // init declarator list
            {
                ASTNode* iter = decl->m_children[0]; 
                while(iter->m_children.size()==2) {
                    iter = iter->m_children[1];
                }
                std::string var = iter->m_children[0]->m_value;

                if(scopes.top().check_decl(var))
                {
                    std::cout << "Variable " << var << " already defined" << std::endl;
                    return false;
                }
                scopes.top().add_symbol(decl->m_value);
            }
            return true;
        case (NodeType::Compound_Statement):
            ASTNode* block = node->m_children[0];
            if(block->m_value == "") return true;

            for(auto child : block->m_children)
            {
                bool flag = check_node(child);
                if(!flag) return false;
            }
            return true;
        case (NodeType::Selection_Statement):
            if(node->m_value == "IF")
            {
                bool check = check_node(node->m_children[0]);
                if(!check) return false;

                enter_scope();
                check = check_node(node->m_children[1]);
                if(!check) return false;
                exit_scope();
            }
            if(node->m_value == "IF ELSE")
            {
                bool check = check_node(node->m_children[0]);
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
                bool check = check_node(node->m_children[0]);
                if(!check) return false;
                enter_scope();
                check = check_node(node->m_children[1]);
                if(!check) return false;
                exit_scope();
            }
            return true;
    }
    return true;
}