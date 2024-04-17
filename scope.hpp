#include "ASTNode.hpp"
#include <stack>
#include <vector>
#include <iostream>


class scope {
    public:
        std::vector<std::string> symbols;
        std::vector<std::string> locals;
    
    scope();

    scope(std::vector<std::string>, std::vector<std::string>);

    void print_symbols();

    void add_symbol(std::string);

    bool check_decl(std::string, bool);

    bool check_scope(std::string, bool);

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