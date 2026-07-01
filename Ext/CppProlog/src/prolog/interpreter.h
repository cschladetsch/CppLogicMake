#pragma once

#include "database.h"
#include "resolver.h"
#include "parser.h"
#include <iostream>
#include <string>
#include <memory>
#include <stdio.h>
#include <stdlib.h>

namespace linenoise {
    class LineNoise;
}

namespace prolog {

class Interpreter {
private:
    std::unique_ptr<Database> database_;
    std::unique_ptr<Resolver> resolver_;
    bool interactive_mode_;
    
public:
    explicit Interpreter(bool interactive = true);
    ~Interpreter() = default;
    
    void run();
    void loadFile(const std::string& filename);
    void loadString(const std::string& program);
    
    std::vector<Solution> query(const std::string& query_string);
    void queryInteractive(const std::string& query_string);
    
    void showHelp() const;
    void showStatistics() const;
    
    Database& database() { return *database_; }
    const Database& database() const { return *database_; }
    
private:
    void processCommand(const std::string& input);
    bool isCommand(const std::string& input) const;
    void handleCommand(const std::string& command);
    
    std::string readInput(const std::string& prompt) const;
    void printSolutions(const std::vector<Solution>& solutions) const;
};

}