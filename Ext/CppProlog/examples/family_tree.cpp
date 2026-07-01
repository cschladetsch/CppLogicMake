#include "prolog/interpreter.h"
#include <iostream>

int main() {
    std::cout << "CppLProlog Family Tree Example\n";
    std::cout << "===============================\n\n";
    
    try {
        prolog::Interpreter interpreter(false);
        
        std::cout << "Loading family tree program from family.pl...\n\n";
        interpreter.loadFile("examples/family.pl");
        
        // Family queries
        std::vector<std::pair<std::string, std::string>> queries = {
            {"Who are tom's children?", "parent(tom, X)"},
            {"Who are the fathers?", "father(X, Y)"},
            {"Who are the grandparents?", "grandparent(X, Y)"},
            {"Who is tom's grandfather of?", "grandfather(tom, X)"},
            {"Who are bob's siblings?", "sibling(bob, X)"},
            {"Who are the uncles?", "uncle(X, Y)"},
            {"Who are cousins?", "cousin(X, Y)"},
            {"Who are tom's descendants?", "descendant(X, tom)"},
            {"Is tom an ancestor of jim?", "ancestor(tom, jim)"},
            {"All parent relationships", "parent(X, Y)"}
        };
        
        for (const auto& [description, query] : queries) {
            std::cout << description << "\n";
            std::cout << "Query: " << query << "\n";
            
            try {
                auto solutions = interpreter.query(query);
                
                if (solutions.empty()) {
                    std::cout << "  Result: No\n";
                } else {
                    std::cout << "  Results:\n";
                    for (size_t i = 0; i < solutions.size(); ++i) {
                        std::cout << "    " << (i + 1) << ". " 
                                  << solutions[i].toString() << "\n";
                    }
                }
            } catch (const std::exception& e) {
                std::cout << "  Error: " << e.what() << "\n";
            }
            
            std::cout << "\n";
        }
        
        // Interactive demonstration
        std::cout << "Family Tree Visualization:\n";
        std::cout << "tom\n";
        std::cout << "├── bob\n";
        std::cout << "│   ├── ann\n";
        std::cout << "│   └── pat\n";
        std::cout << "│       └── jim\n";
        std::cout << "└── liz\n";
        std::cout << "    ├── sue\n";
        std::cout << "    └── joe\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}