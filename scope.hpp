#include "ASTNode.hpp"
#include <stack>
#include <vector>
#include <iostream>
#include <map>


class scope {
    public:
        std::vector<std::string> symbols;
        std::map<std::string, bool> isVariadic;
    
    scope();

    scope(std::vector<std::string>);

    void print_symbols();

    void add_symbol(std::string);

    bool check_scope(std::string);

};

class scopeStack {
    public: 
        int size;
        std::stack<scope> scopes;
        // bool debug;

    scopeStack();

    void enter_scope();

    void exit_scope();

    bool check_node(ASTNode* node);

    
};