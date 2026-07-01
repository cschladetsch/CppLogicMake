#include "prolog/interpreter.h"
#include <iostream>

int main() {
    std::cout << "CppLProlog Basic Example\n";
    std::cout << "========================\n\n";
    
    try {
        prolog::Interpreter interpreter(false);
        
        std::cout << "Loading program from basic.pl...\n\n";
        interpreter.loadFile("examples/basic.pl");
        
        // Example queries
        std::vector<std::string> queries = {
            "likes(mary, wine)",
            "likes(X, wine)",
            "happy(mary)",
            "happy(X)",
            "likes(X, Y)"
        };
        
        for (const auto& query : queries) {
            std::cout << "Query: " << query << "\n";
            
            try {
                auto solutions = interpreter.query(query);
                
                if (solutions.empty()) {
                    std::cout << "  Result: false\n";
                } else {
                    for (size_t i = 0; i < solutions.size(); ++i) {
                        std::cout << "  Solution " << (i + 1) << ": " 
                                  << solutions[i].toString() << "\n";
                    }
                }
            } catch (const std::exception& e) {
                std::cout << "  Error: " << e.what() << "\n";
            }
            
            std::cout << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}