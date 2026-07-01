/**
 * @file main.cpp
 * @brief C++ Project
 * @details Generated using Universal Project Generator
 * 
 * This application demonstrates integration between Prolog-based
 * code generation and C++ application development.
 */

#include <iostream>
#include <string>

int main() {
    std::cout << "⚡ Hello World from C++!" << std::endl;
    std::cout << "This application was generated using:" << std::endl;
    std::cout << "  • CppProlog for rule-based generation" << std::endl;
    std::cout << "  • Rust for the generator system" << std::endl;
    std::cout << "  • Prolog knowledge bases for development files" << std::endl;
    
    std::cout << "Enter your name: ";
    std::string name;
    std::getline(std::cin, name);
    
    if (!name.empty()) {
        std::cout << "Hello, " << name << "! Welcome to the generated C++ application! 🎉" << std::endl;
    } else {
        std::cout << "Hello, anonymous user! Welcome to the generated C++ application! 🎉" << std::endl;
    }
    
    return 0;
}
