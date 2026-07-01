#include "prolog/interpreter.h"
#include <iostream>

int main() {
    std::cout << "CppLProlog List Processing Example\n";
    std::cout << "===================================\n\n";
    
    try {
        prolog::Interpreter interpreter(false);
        
        std::cout << "Loading list processing program from lists.pl...\n\n";
        interpreter.loadFile("examples/lists.pl");
        
        // List processing queries
        std::vector<std::pair<std::string, std::string>> queries = {
            {"Get example list", "example_list(L)"},
            {"Check if 3 is member of [1,2,3,4,5]", "member(3, [1,2,3,4,5])"},
            {"Find all members of [a,b,c]", "member(X, [a,b,c])"},
            {"Append [1,2] and [3,4]", "append([1,2], [3,4], L)"},
            {"Find all ways to split [1,2,3]", "append(X, Y, [1,2,3])"},
            {"Length of [a,b,c,d]", "length([a,b,c,d], N)"},
            {"Reverse [1,2,3,4]", "reverse([1,2,3,4], R)"},
            {"Last element of [a,b,c,d]", "last([a,b,c,d], X)"},
            {"Remove 'b' from [a,b,c,b,d]", "remove(b, [a,b,c,b,d], L)"},
            {"Check if [1,2,3,4] is sorted", "sorted([1,2,3,4])"},
            {"Check if [1,3,2,4] is sorted", "sorted([1,3,2,4])"},
            {"Maximum of [5,2,8,1,9]", "max_list([5,2,8,1,9], M)"},
            {"Process nested list", "nested_list(NL), flatten(NL, FL)"}
        };
        
        for (const auto& [description, query] : queries) {
            std::cout << description << "\n";
            std::cout << "Query: " << query << "\n";
            
            try {
                auto solutions = interpreter.query(query);
                
                if (solutions.empty()) {
                    std::cout << "  Result: false\n";
                } else {
                    if (solutions.size() == 1 && solutions[0].bindings.empty()) {
                        std::cout << "  Result: true\n";
                    } else {
                        std::cout << "  Solutions:\n";
                        for (size_t i = 0; i < solutions.size(); ++i) {
                            std::cout << "    " << (i + 1) << ". " 
                                      << solutions[i].toString() << "\n";
                        }
                    }
                }
            } catch (const std::exception& e) {
                std::cout << "  Error: " << e.what() << "\n";
            }
            
            std::cout << "\n";
        }
        
        // Demonstrate complex list operations
        std::cout << "Complex List Operations Demo:\n";
        std::cout << "-----------------------------\n";
        
        std::vector<std::string> complex_queries = {
            "append([a,b], [c,d], L1), append(L1, [e], L2)",
            "reverse([1,2,3], R), append(R, [4], Final)"
        };
        
        for (const auto& query : complex_queries) {
            std::cout << "Complex query: " << query << "\n";
            
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