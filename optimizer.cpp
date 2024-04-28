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
            if(child_node->m_type == Selection_Statement && child_node->m_value == "IF ELSE"){
                bool left = 0;
                bool right = 0;
                for(int j = 0; j < child_node->m_children[1]->m_children.size(); j++){
                    if(child_node->m_children[1]->m_children[j]->m_type == Jump_Statement && child_node->m_children[1]->m_children[j]->m_value == "RETURN"){
                        left = 1;
                    }
                }
                for(int j = 0; j < child_node->m_children[2]->m_children.size(); j++){
                    if(child_node->m_children[2]->m_children[j]->m_type == Jump_Statement && child_node->m_children[2]->m_children[j]->m_value == "RETURN"){
                        right = 1;
                    }
                }
                std::cout<<"left: " << left << " right: " << right << std::endl;

                if(left && right)
                {
                    while(node->m_children.back() != node->m_children[i]){
                        node->m_children.pop_back();
                    }
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
    switch (node->m_type) {
        case I_Constant:
        case F_Constant:
            return true;
        case Identifier:
            return false;
        case Additive_Expression:
        {
            bool b1 = constant_folding(node->m_children[0], node);
            bool b2 = constant_folding(node->m_children[1], node);
            if(b1 && b2) 
            {
                std::cout << "optimizing " << node->m_children[0]->m_value << node->m_value << node->m_children[1]->m_value << std::endl;
                for (int i = 0; i < parent->m_children.size(); i++) 
                {
                    if (parent->m_children[i] == node) 
                    {
                        if (node->m_value == "+") 
                        {
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) 
                            {
                                parent->m_children[i] = new ASTNode(I_Constant);
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) + std::stoi(node->m_children[1]->m_value));
                            }
                            else
                            {
                                parent->m_children[i] = new ASTNode(F_Constant);
                                parent->m_children[i]->m_value = std::to_string(std::stof(node->m_children[0]->m_value) + std::stof(node->m_children[1]->m_value));
                            } 
                        }
                        else if (node->m_value == "-") 
                        {
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) 
                            {
                                parent->m_children[i] = new ASTNode(I_Constant);
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) - std::stoi(node->m_children[1]->m_value));
                            }
                            else
                            {
                                parent->m_children[i] = new ASTNode(F_Constant);
                                parent->m_children[i]->m_value = std::to_string(std::stof(node->m_children[0]->m_value) - std::stof(node->m_children[1]->m_value));
                            }
                        }
                        return true;
                    }
                }
            }
            else if (b1) 
            {
                if (node->m_value == "+" && node->m_children[0]->m_type == I_Constant && std::stoi(node->m_children[0]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }

                return false;
            }
            else if (b2) 
            {
                if (node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }

                return false;
            }
            return false;
        }
        case Multiplicative_Expression:
        {
            bool b1 = constant_folding(node->m_children[0], node);
            bool b2 = constant_folding(node->m_children[1], node);
            if(b1 && b2) 
            {
                std::cout << "optimizing " << node->m_children[0]->m_value << node->m_value << node->m_children[1]->m_value << std::endl;
                for (int i = 0; i < parent->m_children.size(); i++) 
                {
                    if (parent->m_children[i] == node) 
                    {
                        if (node->m_value == "*") {
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) 
                            {
                                parent->m_children[i] = new ASTNode(I_Constant);
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) * std::stoi(node->m_children[1]->m_value));
                            }
                            else
                            {
                                parent->m_children[i] = new ASTNode(F_Constant);
                                parent->m_children[i]->m_value = std::to_string(std::stof(node->m_children[0]->m_value) * std::stof(node->m_children[1]->m_value));
                            } 
                        }
                        else if (node->m_value == "/") {
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) 
                            {
                                parent->m_children[i] = new ASTNode(I_Constant);
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) / std::stoi(node->m_children[1]->m_value));
                            }
                            else
                            {
                                parent->m_children[i] = new ASTNode(F_Constant);
                                parent->m_children[i]->m_value = std::to_string(std::stof(node->m_children[0]->m_value) / std::stof(node->m_children[1]->m_value));
                            }
                        }
                        else if (node->m_value == "%") {
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) 
                            {
                                parent->m_children[i] = new ASTNode(I_Constant);
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) % std::stoi(node->m_children[1]->m_value));
                            }
                            else std::cerr << "Modulo operation doesn't support floating operands" << std::endl;
                        }
                        return true;
                    }
                }
            }
            else if (b1) 
            {
                if (node->m_value == "*" && node->m_children[0]->m_type == I_Constant && std::stoi(node->m_children[0]->m_value) == 1) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }
                if (node->m_value == "*" && node->m_children[0]->m_type == I_Constant && std::stoi(node->m_children[0]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }
                if (node->m_value == "*" && node->m_children[0]->m_type == F_Constant && std::stoi(node->m_children[0]->m_value) == 0.0f) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }

                return false;
            }
            else if (b2) 
            {
                if (node->m_value == "*" && node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 1) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }
                if (node->m_value == "*" && node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }
                if (node->m_value == "*" && node->m_children[1]->m_type == F_Constant && std::stoi(node->m_children[1]->m_value) == 0.0f) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }
                if (node->m_value == "/" && node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 1) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }

                return false;
            }
            return false;
        }
        case Shift_Expression:
        {
            bool b1 = constant_folding(node->m_children[0], node);
            bool b2 = constant_folding(node->m_children[1], node);
            if(b1 && b2) 
            {
                std::cout << "optimizing " << node->m_children[0]->m_value << node->m_value << node->m_children[1]->m_value << std::endl;
                for (int i = 0; i < parent->m_children.size(); i++) 
                {
                    if (parent->m_children[i] == node) 
                    {
                        if (node->m_value == "<<") 
                        {
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) 
                            {
                                parent->m_children[i] = new ASTNode(I_Constant);
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) << std::stoi(node->m_children[1]->m_value));
                            }
                            else std::cerr << "<< operation doesn't support floating operands" << std::endl;
                        }
                        else if (node->m_value == ">>") 
                        {
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) 
                            {
                                parent->m_children[i] = new ASTNode(I_Constant);
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) >> std::stoi(node->m_children[1]->m_value));
                            }
                            else std::cerr << ">> operation doesn't support floating operands" << std::endl;
                        }
                        return true;
                    }
                }
            }
            else if (b1) 
            {
                if (node->m_children[0]->m_type == I_Constant && std::stoi(node->m_children[0]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }
                
                return false;
            }
            else if (b2) 
            {
                if (node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }

                return false;
            }
            return false;
        }
        case Relational_Expression:
        {
            bool b1 = constant_folding(node->m_children[0], node);
            bool b2 = constant_folding(node->m_children[1], node);
            if(b1 && b2) 
            {
                std::cout << "optimizing " << node->m_children[0]->m_value << node->m_value << node->m_children[1]->m_value << std::endl;
                for (int i = 0; i < parent->m_children.size(); i++) 
                {
                    if (parent->m_children[i] == node) 
                    {
                        if (node->m_value == ">") 
                        {
                            parent->m_children[i] = new ASTNode(I_Constant);
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) {
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) > std::stoi(node->m_children[1]->m_value));
                            }
                            else parent->m_children[i]->m_value = std::to_string(std::stof(node->m_children[0]->m_value) > std::stof(node->m_children[1]->m_value));
                        }
                        else if (node->m_value == "<") 
                        {
                            parent->m_children[i] = new ASTNode(I_Constant);
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) {
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) < std::stoi(node->m_children[1]->m_value));
                            }
                            else parent->m_children[i]->m_value = std::to_string(std::stof(node->m_children[0]->m_value) < std::stof(node->m_children[1]->m_value));
                        }
                        else if (node->m_value == ">=") 
                        {
                            parent->m_children[i] = new ASTNode(I_Constant);
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) {
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) >= std::stoi(node->m_children[1]->m_value));
                            }
                            else parent->m_children[i]->m_value = std::to_string(std::stof(node->m_children[0]->m_value) >= std::stof(node->m_children[1]->m_value));
                        }
                        else if (node->m_value == "<=") 
                        {
                            parent->m_children[i] = new ASTNode(I_Constant);
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) {
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) <= std::stoi(node->m_children[1]->m_value));
                            }
                            else parent->m_children[i]->m_value = std::to_string(std::stof(node->m_children[0]->m_value) <= std::stof(node->m_children[1]->m_value));
                        }
                        return true;
                    }
                }
            }
            return false;
        }
        case Equality_Expression:
        {
            bool b1 = constant_folding(node->m_children[0], node);
            bool b2 = constant_folding(node->m_children[1], node);
            if(b1 && b2) 
            {
                std::cout << "optimizing " << node->m_children[0]->m_value << node->m_value << node->m_children[1]->m_value << std::endl;
                for (int i = 0; i < parent->m_children.size(); i++) 
                {
                    if (parent->m_children[i] == node) 
                    {
                        if (node->m_value == "==") 
                        {
                            parent->m_children[i] = new ASTNode(I_Constant);
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) {
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) == std::stoi(node->m_children[1]->m_value));
                            }
                            else parent->m_children[i]->m_value = std::to_string(std::stof(node->m_children[0]->m_value) == std::stof(node->m_children[1]->m_value));
                        }
                        else if (node->m_value == "!=") 
                        {
                            parent->m_children[i] = new ASTNode(I_Constant);
                            if(node->m_children[0]->m_type == I_Constant && node->m_children[1]->m_type == I_Constant) {
                                parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) != std::stoi(node->m_children[1]->m_value));
                            }
                            else parent->m_children[i]->m_value = std::to_string(std::stof(node->m_children[0]->m_value) != std::stof(node->m_children[1]->m_value));
                        }
                        return true;
                    }
                }
            }
            return false;
        }
        case And_Expression:
        {
            bool b1 = constant_folding(node->m_children[0], node);
            bool b2 = constant_folding(node->m_children[1], node);
            if(b1 && b2) 
            {
                std::cout << "optimizing " << node->m_children[0]->m_value << node->m_value << node->m_children[1]->m_value << std::endl;
                for (int i = 0; i < parent->m_children.size(); i++) 
                {
                    if (parent->m_children[i] == node) 
                    {
                        parent->m_children[i] = new ASTNode(I_Constant);
                        parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) & std::stoi(node->m_children[1]->m_value));
                        return true;
                    }
                }
            }
            else if(b1)
            {
                if (node->m_children[0]->m_type == I_Constant && std::stoi(node->m_children[0]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }
                return false;
            }
            else if(b2)
            {
                if (node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }
                return false;
            }
            return false;
        }
        case Exclusive_Or_Expression:
        {
            bool b1 = constant_folding(node->m_children[0], node);
            bool b2 = constant_folding(node->m_children[1], node);
            if(b1 && b2) 
            {
                std::cout << "optimizing " << node->m_children[0]->m_value << node->m_value << node->m_children[1]->m_value << std::endl;
                for (int i = 0; i < parent->m_children.size(); i++) 
                {
                    if (parent->m_children[i] == node) 
                    {
                        parent->m_children[i] = new ASTNode(I_Constant);
                        parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) ^ std::stoi(node->m_children[1]->m_value));
                        return true;
                    }
                }
            }
            else if(b1)
            {
                if (node->m_children[0]->m_type == I_Constant && std::stoi(node->m_children[0]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }
                return false;
            }
            else if(b2)
            {
                if (node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }
                return false;
            }
            return false;
        }
        case Inclusive_Or_Expression:
        {
            bool b1 = constant_folding(node->m_children[0], node);
            bool b2 = constant_folding(node->m_children[1], node);
            if(b1 && b2) 
            {
                std::cout << "optimizing " << node->m_children[0]->m_value << node->m_value << node->m_children[1]->m_value << std::endl;
                for (int i = 0; i < parent->m_children.size(); i++) 
                {
                    if (parent->m_children[i] == node) 
                    {
                        parent->m_children[i] = new ASTNode(I_Constant);
                        parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) | std::stoi(node->m_children[1]->m_value));
                        return true;
                    }
                }
            }
            else if(b1)
            {
                if (node->m_children[0]->m_type == I_Constant && std::stoi(node->m_children[0]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }
                return false;
            }
            else if(b2)
            {
                if (node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }
                return false;
            }
            return false;
        }
        case Logical_And_Expression:
        {
            bool b1 = constant_folding(node->m_children[0], node);
            bool b2 = constant_folding(node->m_children[1], node);
            if(b1 && b2) 
            {
                std::cout << "optimizing " << node->m_children[0]->m_value << "&&" << node->m_children[1]->m_value << std::endl;
                for (int i = 0; i < parent->m_children.size(); i++) 
                {
                    if (parent->m_children[i] == node) 
                    {
                        parent->m_children[i] = new ASTNode(I_Constant);
                        parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) && std::stoi(node->m_children[1]->m_value));
                        return true;
                    }
                }
            }
            else if(b1)
            {
                if (node->m_children[0]->m_type == I_Constant && std::stoi(node->m_children[0]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }
                if(node->m_children[0]->m_type == I_Constant && std::stoi(node->m_children[0]->m_value) == 1) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }
                return false;
            }
            else if(b2)
            {
                if (node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }
                if(node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 1) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }
                return false;
            }
            return false;
        }
        case Logical_Or_Expression:
        {
            bool b1 = constant_folding(node->m_children[0], node);
            bool b2 = constant_folding(node->m_children[1], node);
            if(b1 && b2) 
            {
                std::cout << "optimizing " << node->m_children[0]->m_value << "||" << node->m_children[1]->m_value << std::endl;
                for (int i = 0; i < parent->m_children.size(); i++) 
                {
                    if (parent->m_children[i] == node) 
                    {
                        parent->m_children[i] = new ASTNode(I_Constant);
                        parent->m_children[i]->m_value = std::to_string(std::stoi(node->m_children[0]->m_value) || std::stoi(node->m_children[1]->m_value));
                        return true;
                    }
                }
            }
            else if(b1)
            {
                if (node->m_children[0]->m_type == I_Constant && std::stoi(node->m_children[0]->m_value) == 1) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }
                if(node->m_children[0]->m_type == I_Constant && std::stoi(node->m_children[0]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }
                return false;
            }
            else if(b2)
            {
                if (node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 1) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[1];
                        }
                    }
                }
                if(node->m_children[1]->m_type == I_Constant && std::stoi(node->m_children[1]->m_value) == 0) {
                    std::cout << "optimizing " << node->m_children[1]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) {
                        if (parent->m_children[i] == node) {
                            parent->m_children[i] = node->m_children[0];
                        }
                    }
                }
                return false;
            }
            return false;
        }
        case Conditional_Expression:
        {
            bool b1 = constant_folding(node->m_children[0], node);
            bool b2 = constant_folding(node->m_children[1], node);
            bool b3 = constant_folding(node->m_children[2], node);
            if(b1) 
            {
                std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                for (int i = 0; i < parent->m_children.size(); i++) 
                {
                    if (parent->m_children[i] == node) 
                    {
                        if(std::stof(node->m_children[0]->m_value) != 0.0f) parent->m_children[i] = node->m_children[1];
                        else parent->m_children[i] = node->m_children[2];
                        return true;
                    }
                }
            }
            return false;
        }
        case Selection_Statement:
        {
            if(node->m_value == "IF")
            {
                bool b1 = constant_folding(node->m_children[0], node);
                bool b2 = constant_folding(node->m_children[1], node);
                if(b1) 
                {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) 
                    {
                        if (parent->m_children[i] == node) 
                        {
                            if(std::stof(node->m_children[0]->m_value) != 0.0f) parent->m_children[i] = node->m_children[1];
                            else parent->m_children.erase(std::find(parent->m_children.begin(), parent->m_children.end(), node));
                        }
                    }
                }
            }
            else if(node->m_value == "IF ELSE")
            {
                bool b1 = constant_folding(node->m_children[0], node);
                bool b2 = constant_folding(node->m_children[1], node);
                bool b3 = constant_folding(node->m_children[2], node);
                if(b1) 
                {
                    std::cout << "optimizing " << node->m_children[0]->m_value << std::endl;
                    for (int i = 0; i < parent->m_children.size(); i++) 
                    {
                        if (parent->m_children[i] == node) 
                        {
                            if(std::stof(node->m_children[0]->m_value) != 0.0f) parent->m_children[i] = node->m_children[1];
                            else parent->m_children[i] = node->m_children[2];
                        }
                    }
                }
            }
            return false;
        }
        default:
        {    
            int size = node->m_children.size();
            int i = 0;
            while(i < node->m_children.size()) {
                constant_folding(node->m_children[i], node);
                if(size != node->m_children.size()) size--;
                else i++;
            }
            return false;
        }
    }
}