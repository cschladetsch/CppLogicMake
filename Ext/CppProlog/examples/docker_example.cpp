#include "prolog/interpreter.h"
#include <iostream>
#include <fstream>

int main() {
    std::cout << "CppLProlog Docker Generator Example\n";
    std::cout << "===================================\n\n";
    
    try {
        prolog::Interpreter interpreter(false);
        
        // Load the Docker generator Prolog program
        std::cout << "Loading Docker generator program...\n";
        interpreter.loadFile("examples/docker_minimal.pl");
        
        std::cout << "\n1. Generating Production Dockerfile:\n";
        std::cout << "=====================================\n";
        
        // Generate production Dockerfile
        auto production_result = interpreter.query("cppprolog_dockerfile(production)");
        
        std::cout << "\n2. Analyzing Security Issues:\n";
        std::cout << "=============================\n";
        
        // Check for security issues in production strategy
        auto security_result = interpreter.query("build_strategy(optimized_production, S), security_check(S, Issues)");
        
        if (!security_result.empty()) {
            std::cout << "Security analysis completed. Found issues: ";
            auto issues_binding = security_result[0].bindings.find("Issues");
            if (issues_binding != security_result[0].bindings.end()) {
                std::cout << issues_binding->second->toString() << std::endl;
            }
        }
        
        std::cout << "\n3. Build Performance Optimization:\n";
        std::cout << "===================================\n";
        
        // Get optimized build strategy
        auto optimization_result = interpreter.query("optimize_build_performance(optimized_production, Optimized)");
        
        if (!optimization_result.empty()) {
            std::cout << "Build optimization strategies available.\n";
        }
        
        std::cout << "\n4. Package Analysis:\n";
        std::cout << "====================\n";
        
        // Analyze essential packages
        auto packages_result = interpreter.query("essential_build_packages(Packages)");
        if (!packages_result.empty()) {
            auto packages_binding = packages_result[0].bindings.find("Packages");
            if (packages_binding != packages_result[0].bindings.end()) {
                std::cout << "Essential build packages: " << packages_binding->second->toString() << std::endl;
            }
        }
        
        // Estimate image size
        auto size_result = interpreter.query("essential_build_packages(P), estimate_image_size(P, Size)");
        if (!size_result.empty()) {
            auto size_binding = size_result[0].bindings.find("Size");
            if (size_binding != size_result[0].bindings.end()) {
                std::cout << "Estimated image size: " << size_binding->second->toString() << " MB" << std::endl;
            }
        }
        
        std::cout << "\n5. CI/CD Dockerfile Generation:\n";
        std::cout << "===============================\n";
        
        // Generate CI/CD Dockerfile
        auto ci_result = interpreter.query("ci_dockerfile");
        
        std::cout << "\n6. Docker Instruction Validation:\n";
        std::cout << "=================================\n";
        
        // Test instruction validation
        auto validation_result = interpreter.query("docker_instruction(from('ubuntu:22.04'))");
        if (!validation_result.empty()) {
            std::cout << "✓ FROM ubuntu:22.04 - Valid instruction\n";
        }
        
        auto invalid_result = interpreter.query("docker_instruction(invalid_instruction)");
        if (invalid_result.empty()) {
            std::cout << "✗ invalid_instruction - Invalid instruction (correctly rejected)\n";
        }
        
        std::cout << "\n7. Build Strategy Comparison:\n";
        std::cout << "============================\n";
        
        // Compare different build strategies
        std::vector<std::string> strategies = {"fast_development", "optimized_production", "ci_testing"};
        
        for (const auto& strategy : strategies) {
            std::cout << "\nStrategy: " << strategy << std::endl;
            auto strategy_query = "build_strategy(" + strategy + ", Instructions)";
            auto strategy_result = interpreter.query(strategy_query);
            
            if (!strategy_result.empty()) {
                std::cout << "✓ Strategy defined and available" << std::endl;
            } else {
                std::cout << "✗ Strategy not found" << std::endl;
            }
        }
        
        std::cout << "\n8. Advanced Features Demonstration:\n";
        std::cout << "===================================\n";
        
        // Demonstrate layer optimization
        std::cout << "Testing layer optimization rules...\n";
        auto layer_opt_result = interpreter.query("cache_optimized_layers(builder, Instructions)");
        if (!layer_opt_result.empty()) {
            std::cout << "✓ Layer optimization rules working\n";
        }
        
        // Demonstrate security optimization  
        std::cout << "Testing security optimization rules...\n";
        auto security_opt_result = interpreter.query("security_optimized(Instructions)");
        if (!security_opt_result.empty()) {
            std::cout << "✓ Security optimization rules working\n";
        }
        
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "Docker Generator Demo Complete!" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        std::cout << "\nTo generate a Dockerfile interactively, you can run:" << std::endl;
        std::cout << "./bin/prolog_interpreter examples/docker_generator.pl" << std::endl;
        std::cout << "?- build_interactive_dockerfile." << std::endl;
        
        std::cout << "\nOr generate specific types programmatically:" << std::endl;
        std::cout << "?- cppprolog_dockerfile(production)." << std::endl;
        std::cout << "?- cppprolog_dockerfile(development)." << std::endl;
        std::cout << "?- ci_dockerfile." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}