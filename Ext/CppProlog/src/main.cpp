#include "prolog/interpreter.h"
#include <rang.hpp>
#include <iostream>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
    try {
        prolog::Interpreter interpreter(true);
        
        std::vector<std::string> files;
        bool interactive = true;
        std::string query;
        
        // Parse command line arguments
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "-h" || arg == "--help") {
                std::cout << rang::style::bold << rang::fg::cyan 
                          << "CppLProlog - A modern Prolog interpreter" << rang::style::reset << "\n\n";
                std::cout << rang::style::bold << "Usage: " << rang::style::reset 
                          << argv[0] << " [options] [files...]\n\n";
                std::cout << rang::style::bold << "Options:" << rang::style::reset << "\n";
                std::cout << "  " << rang::fg::green << "-h, --help" << rang::style::reset 
                          << "     Show this help message\n";
                std::cout << "  " << rang::fg::green << "-q, --query Q" << rang::style::reset 
                          << "  Execute query Q and exit\n";
                std::cout << "  " << rang::fg::green << "-n, --no-interactive" << rang::style::reset 
                          << "  Don't start interactive mode\n";
                std::cout << "\n" << rang::style::bold << "Files:" << rang::style::reset << "\n";
                std::cout << "  Any .pl files will be loaded before starting\n";
                return 0;
            } else if (arg == "-q" || arg == "--query") {
                if (i + 1 < argc) {
                    query = argv[++i];
                    interactive = false;
                } else {
                    std::cerr << "Error: --query requires an argument\n";
                    return 1;
                }
            } else if (arg == "-n" || arg == "--no-interactive") {
                interactive = false;
            } else if (arg.length() >= 3 && (arg.substr(arg.length() - 3) == ".pl" || 
                                              arg.length() >= 4 && arg.substr(arg.length() - 4) == ".pro")) {
                files.push_back(arg);
            } else {
                std::cerr << "Unknown argument: " << arg << "\n";
                return 1;
            }
        }
        
        // Load files
        for (const auto& file : files) {
            std::cout << rang::fg::yellow << "Loading " << file << "..." << rang::style::reset << "\n";
            try {
                interpreter.loadFile(file);
                std::cout << rang::fg::green << "Loaded successfully." << rang::style::reset << "\n";
            } catch (const std::exception& e) {
                std::cerr << rang::fg::red << "Error loading " << file << ": " << e.what() 
                          << rang::style::reset << "\n";
                return 1;
            }
        }
        
        // Execute query if provided
        if (!query.empty()) {
            try {
                auto solutions = interpreter.query(query);
                if (solutions.empty()) {
                    std::cout << "false.\n";
                } else {
                    for (const auto& solution : solutions) {
                        std::cout << solution.toString() << "\n";
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Query error: " << e.what() << "\n";
                return 1;
            }
        }
        
        // Start interactive mode if requested
        if (interactive) {
            interpreter.run();
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}