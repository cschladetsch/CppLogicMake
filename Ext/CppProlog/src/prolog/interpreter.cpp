#include "interpreter.h"
#include "builtin_predicates.h"
#include <rang.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

namespace prolog {

Interpreter::Interpreter(bool interactive) 
    : database_(std::make_unique<Database>()),
      resolver_(std::make_unique<Resolver>(*database_)),
      interactive_mode_(interactive) {
    BuiltinPredicates::registerBuiltins();
}

void Interpreter::run() {
    if (!interactive_mode_) return;
    
    std::cout << rang::style::bold << rang::fg::cyan 
              << "CppLProlog Interpreter v1.0" << rang::style::reset << "\n";
    std::cout << "Type " << rang::fg::yellow << ":help" << rang::style::reset 
              << " for commands, or enter Prolog queries.\n\n";
    
    while (true) {
        std::string input = readInput("?- ");
        
        if (input.empty()) continue;
        
        if (input == ":quit" || input == ":q") {
            break;
        }
        
        try {
            if (isCommand(input)) {
                handleCommand(input);
            } else if (input.back() == '.') {
                // Looks like a clause, add to database
                database_->loadProgram(input);
                std::cout << "Clause added.\n";
            } else {
                // Treat as query
                queryInteractive(input);
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
        
        std::cout << "\n";
    }
    
    std::cout << "Goodbye!\n";
}

void Interpreter::loadFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    
    database_->loadProgram(content.str());
}

void Interpreter::loadString(const std::string& program) {
    database_->loadProgram(program);
}

std::vector<Solution> Interpreter::query(const std::string& query_string) {
    Parser parser({});
    TermPtr query = parser.parseQuery(query_string);
    
    return resolver_->solve(query);
}

void Interpreter::queryInteractive(const std::string& query_string) {
    try {
        auto solutions = query(query_string);
        printSolutions(solutions);
    } catch (const std::exception& e) {
        std::cout << "Query error: " << e.what() << "\n";
    }
}

void Interpreter::showHelp() const {
    std::cout << "Commands:\n";
    std::cout << "  :help, :h     - Show this help\n";
    std::cout << "  :quit, :q     - Exit interpreter\n";
    std::cout << "  :load <file>  - Load Prolog file\n";
    std::cout << "  :clear        - Clear database\n";
    std::cout << "  :list         - List all clauses\n";
    std::cout << "  :stats        - Show statistics\n";
    std::cout << "\nQuery examples:\n";
    std::cout << "  parent(tom, bob).\n";
    std::cout << "  parent(X, bob).\n";
}

void Interpreter::showStatistics() const {
    std::cout << "Database statistics:\n";
    std::cout << "  Clauses: " << database_->size() << "\n";
}

void Interpreter::processCommand(const std::string& input) {
    if (isCommand(input)) {
        handleCommand(input);
    }
}

bool Interpreter::isCommand(const std::string& input) const {
    return !input.empty() && input[0] == ':';
}

void Interpreter::handleCommand(const std::string& command) {
    if (command == ":help" || command == ":h") {
        showHelp();
    } else if (command == ":clear") {
        database_->clear();
        std::cout << "Database cleared.\n";
    } else if (command == ":list") {
        std::cout << database_->toString();
    } else if (command == ":stats") {
        showStatistics();
    } else if (command.substr(0, 5) == ":load") {
        if (command.length() > 6) {
            std::string filename = command.substr(6);
            loadFile(filename);
            std::cout << "Loaded file: " << filename << "\n";
        } else {
            std::cout << "Usage: :load <filename>\n";
        }
    } else {
        std::cout << "Unknown command: " << command << "\n";
        std::cout << "Type :help for available commands.\n";
    }
}

std::string Interpreter::readInput(const std::string& prompt) const {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

void Interpreter::printSolutions(const std::vector<Solution>& solutions) const {
    if (solutions.empty()) {
        std::cout << "false.\n";
    } else {
        for (size_t i = 0; i < solutions.size(); ++i) {
            std::cout << solutions[i].toString();
            if (i < solutions.size() - 1) {
                std::cout << " ;";
            }
            std::cout << "\n";
        }
        if (solutions.size() == 1 && solutions[0].bindings.empty()) {
            std::cout << "true.\n";
        }
    }
}

}