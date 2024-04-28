#include "optimizer.hpp"
#include <algorithm>

bool returns(ASTNode* node) {
    for(int i = 0; i < node->m_children.size(); i++) {
        if (node->m_children[i]->m_type == Jump_Statement && node->m_value == "RETURN"){
            return true;
        }
    }
    return false;
}

void dce(ASTNode* node){
    if (node->m_type == Block){
        for(int i = 0; i < node->m_children.size(); i++){
            ASTNode* child_node = node->m_children[i];
            dce(child_node);
            if ((child_node->m_type == Jump_Statement && child_node->m_value == "RETURN") || 
                (child_node->m_type == Block && returns(node))){
                    while(node->m_children.back() != node->m_children[i]){
                        node->m_children.pop_back();
                    }
            }
        }
    }

    else if (node->m_children.size() > 0){
        for (int i = 0; i < node->m_children.size(); i++){
            dce(node->m_children[i]);
        }
    }
}

bool constant_folding(ASTNode* node, ASTNode* parent = nullptr) {

    // for(int i = 0; i < parent_node->m_children.size(); i++) {
        // ASTNode* node = parent_node->m_children[i];

        if (node->m_type == I_Constant || node->m_type == F_Constant) {
            return true;
        } 

        else if (node->m_type == Identifier) {
            return false;
        }

        else if (node->m_type == Additive_Expression){
            if (node->m_children.size() == 2){
                bool b1 = constant_folding(node->m_children[0], node);
                bool b2 = constant_folding(node->m_children[1], node);
                if (b1){
                    if (node->m_value == "+" && std::stoi(node->m_children[0]->m_value) == 0) {
                        std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[1];
                            }
                        }
                    }
                    return false;
                }

                else if (b2){
                    if (node->m_value == "+" && std::stoi(node->m_children[1]->m_value) == 0) {
                        std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[0];
                            }
                        }
                    }
                    if (node->m_value == "-" && std::stoi(node->m_children[1]->m_value) == 0) {
                        std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[0];
                            }
                        }
                    }                 
                }
            }
        }


        else if (node->m_type == Multiplicative_Expression){
            if (node->m_children.size() == 2){
                bool b1 = constant_folding(node->m_children[0], node);
                bool b2 = constant_folding(node->m_children[1], node);
                if (b1){
                    if (node->m_value == "*" && std::stoi(node->m_children[0]->m_value) == 1) {
                        std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[1];
                            }
                        }
                    }

                    else if (node->m_value == "*" && std::stoi(node->m_children[0]->m_value) == 0) {
                        std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[0];
                            }
                        }
                    }

                }

                else if (b2){
                    if (node->m_value == "*" && std::stoi(node->m_children[1]->m_value) == 1) {
                        std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[0];
                            }
                        }
                    }
                    else if (node->m_value == "/" && std::stoi(node->m_children[1]->m_value) == 1) {
                        std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[0];
                            }
                        }
                    }                 

                    else if (node->m_value == "*" && std::stoi(node->m_children[1]->m_value) == 0) {
                        std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[1];
                            }
                        }
                    }
                }
            }
        }


        else if (node->m_type == Logical_And_Expression){
            if (node->m_children.size() == 2){
                bool b1 = constant_folding(node->m_children[0], node);
                bool b2 = constant_folding(node->m_children[1], node);
                if (b1 && b2) {
                    int op1 = std::stoi(node->m_children[0]->m_value);
                    int op2 = std::stoi(node->m_children[1]->m_value);
                    
                    ASTNode* newnode = new ASTNode(I_Constant);
                    if (op1 && op2){
                        newnode->m_value = "1";
                    }
                    else{
                        newnode->m_value = "0";
                    }

                    for(int i = 0; i < parent->m_children.size(); i++){
                        if (parent->m_children[i] == node){
                            parent->m_children[i] = newnode;
                            return true;
                        }
                    }                    
                }

                else if (b1){
                    if (std::stoi(node->m_children[0]->m_value) == 1) {

                        std::cout << "optimizing here" << std::endl;
                        std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            std::cout << "optimizing here" << std::endl;
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[1];
                            }
                        }
                    }

                    else if (std::stoi(node->m_children[0]->m_value) == 0) {
                        std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[0];
                            }
                        }
                    }

                }

                else if (b2){
                    if (std::stoi(node->m_children[1]->m_value) == 1) {
                        std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[0];
                            }
                        }
                    }
                    else if (std::stoi(node->m_children[1]->m_value) == 0) {
                        std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                        for(int i = 0; i < parent->m_children.size(); i++){
                            if (parent->m_children[i] == node){
                                parent->m_children[i] = node->m_children[1];
                            }
                        }
                    }                 
                }
            }
        }




        else if (node->m_type == Relational_Expression || node->m_type == Equality_Expression){
            if (node->m_children.size() == 2){
                bool b1 = constant_folding(node->m_children[0], node);
                bool b2 = constant_folding(node->m_children[1], node);
                if (b1 && b2){
                    if (node->m_value == ">"){
                        int x1 = std::stoi(node->m_children[0]->m_value);
                        int x2 = std::stoi(node->m_children[1]->m_value); 
                        if (x1 > x2){
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "1";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                        else {
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "0";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                    }



                    if (node->m_value == "<"){
                        int x1 = std::stoi(node->m_children[0]->m_value);
                        int x2 = std::stoi(node->m_children[1]->m_value); 
                        if (x1 < x2){
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "1";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                        else {
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "0";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                    }


                    if (node->m_value == ">="){
                        int x1 = std::stoi(node->m_children[0]->m_value);
                        int x2 = std::stoi(node->m_children[1]->m_value); 
                        if (x1 >= x2){
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "1";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                        else {
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "0";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                    }


                    if (node->m_value == "<="){
                        int x1 = std::stoi(node->m_children[0]->m_value);
                        int x2 = std::stoi(node->m_children[1]->m_value); 
                        if (x1 <= x2){
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "1";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                        else {
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "0";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                    }


                    if (node->m_value == "=="){
                        int x1 = std::stoi(node->m_children[0]->m_value);
                        int x2 = std::stoi(node->m_children[1]->m_value); 
                        if (x1 == x2){
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "1";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                        else {
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "0";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                    }


                    if (node->m_value == "!="){
                        int x1 = std::stoi(node->m_children[0]->m_value);
                        int x2 = std::stoi(node->m_children[1]->m_value); 
                        if (x1 != x2){
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "1";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                        else {
                            ASTNode* newnode = new ASTNode(I_Constant);
                            newnode->m_value = "0";

                            for(int i = 0; i < parent->m_children.size(); i++){
                                if (parent->m_children[i] == node){
                                    parent->m_children[i] = newnode;
                                    return true;
                                }
                            }
                        }
                    }

                    
                }
            }
        }

        else if (node->m_type == Selection_Statement && node->m_value == "IF ELSE"){
            bool b = constant_folding(node->m_children[0], node);
            if (b){
                if (std::stoi(node->m_children[0]->m_value) != 0){
                    for(int i = 0; i < parent->m_children.size(); i++){
                        if (parent->m_children[i] == node){
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }
                else if (std::stoi(node->m_children[0]->m_value) == 0){
                    for(int i = 0; i < parent->m_children.size(); i++){
                        if (parent->m_children[i] == node){
                            parent->m_children[i] = node->m_children[2];
                        }
                    }
                }
            }
        }

        else if (node->m_type == Selection_Statement && node->m_value == "IF"){
            bool b = constant_folding(node->m_children[0], node);
            std::cout << "trying to optimize if statement" << std::endl;
            if (node->m_children[0]->m_type == I_Constant){
                std::cout << "integer constant condition" << std::endl;
            }
            if (b){
                std::cout << "optimization possible" << std::endl;
                if (std::stoi(node->m_children[0]->m_value) != 0){
                    for(int i = 0; i < parent->m_children.size(); i++){
                        if (parent->m_children[i] == node){
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }

                else if (std::stoi(node->m_children[0]->m_value) == 0){
                    // for(int i = 0; i < parent->m_children.size(); i++){
                    //     if (parent->m_children[i] == node){
                    //         parent->m_children[i] = node->m_children[2];
                    //     }
                    // }
                    std::cout << "erasing element" << std::endl;
                    parent->m_children.erase(std::find(parent->m_children.begin(), parent->m_children.end(), node));
                }
            }
        }
  
        else {
            for(int i = 0; i < node->m_children.size(); i++) {
                constant_folding(node->m_children[i], node);
            }
            return false;
        }
        return false;
    // }
}