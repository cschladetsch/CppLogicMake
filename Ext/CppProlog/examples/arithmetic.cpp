#include "prolog/interpreter.h"
#include <iostream>

int main() {
    std::cout << "CppLProlog Arithmetic Example\n";
    std::cout << "==============================\n\n";
    
    try {
        prolog::Interpreter interpreter(false);
        
        // Arithmetic program
        std::string program = R"(
            % Basic arithmetic facts
            add(X, Y, Z) :- Z is X + Y.
            subtract(X, Y, Z) :- Z is X - Y.
            multiply(X, Y, Z) :- Z is X * Y.
            divide(X, Y, Z) :- Z is X / Y.
            
            % Factorial
            factorial(0, 1).
            factorial(N, F) :- 
                N > 0,
                N1 is N - 1,
                factorial(N1, F1),
                F is N * F1.
            
            % Fibonacci
            fibonacci(0, 0).
            fibonacci(1, 1).
            fibonacci(N, F) :-
                N > 1,
                N1 is N - 1,
                N2 is N - 2,
                fibonacci(N1, F1),
                fibonacci(N2, F2),
                F is F1 + F2.
            
            % Greatest Common Divisor (Euclidean algorithm)
            gcd(X, 0, X) :- X > 0.
            gcd(X, Y, G) :-
                Y > 0,
                R is X mod Y,
                gcd(Y, R, G).
            
            % Check if number is even/odd
            even(X) :- 0 is X mod 2.
            odd(X) :- 1 is X mod 2.
            
            % Check if number is prime
            is_prime(2).
            is_prime(N) :-
                N > 2,
                odd(N),
                \+ has_divisor(N, 3).
            
            has_divisor(N, D) :-
                D * D =< N,
                0 is N mod D.
            has_divisor(N, D) :-
                D * D =< N,
                D1 is D + 2,
                has_divisor(N, D1).
            
            % Sum of list
            sum_list([], 0).
            sum_list([H|T], Sum) :-
                sum_list(T, TailSum),
                Sum is H + TailSum.
            
            % Range generation
            range(Start, End, []) :- Start > End.
            range(Start, End, [Start|Rest]) :-
                Start =< End,
                Next is Start + 1,
                range(Next, End, Rest).
            
            % Power function
            power(_, 0, 1).
            power(Base, Exp, Result) :-
                Exp > 0,
                Exp1 is Exp - 1,
                power(Base, Exp1, Result1),
                Result is Base * Result1.
            
            % Absolute value
            abs(X, X) :- X >= 0.
            abs(X, AbsX) :- X < 0, AbsX is -X.
            
            % Square root (Newton's method approximation)
            sqrt_approx(X, Root) :- sqrt_iter(X, X, Root).
            
            sqrt_iter(X, Guess, Root) :-
                NewGuess is (Guess + X/Guess) / 2,
                abs(Guess - NewGuess, Diff),
                (   Diff < 0.001 -> Root = NewGuess
                ;   sqrt_iter(X, NewGuess, Root)
                ).
        )";
        
        std::cout << "Loading arithmetic program...\n\n";
        interpreter.loadString(program);
        
        // Arithmetic queries
        std::vector<std::pair<std::string, std::string>> queries = {
            {"Basic addition: 5 + 3", "add(5, 3, X)"},
            {"Basic subtraction: 10 - 4", "subtract(10, 4, X)"},
            {"Basic multiplication: 6 * 7", "multiply(6, 7, X)"},
            {"Basic division: 15 / 3", "divide(15, 3, X)"},
            {"Factorial of 5", "factorial(5, X)"},
            {"Fibonacci of 8", "fibonacci(8, X)"},
            {"GCD of 48 and 18", "gcd(48, 18, X)"},
            {"Is 7 even?", "even(7)"},
            {"Is 8 even?", "even(8)"},
            {"Is 17 prime?", "is_prime(17)"},
            {"Is 21 prime?", "is_prime(21)"},
            {"Sum of [1,2,3,4,5]", "sum_list([1,2,3,4,5], X)"},
            {"Generate range 1 to 5", "range(1, 5, X)"},
            {"2 to the power of 8", "power(2, 8, X)"},
            {"Absolute value of -42", "abs(-42, X)"},
            {"Square root of 16 (approx)", "sqrt_approx(16, X)"}
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
                        for (size_t i = 0; i < solutions.size(); ++i) {
                            std::cout << "  Solution " << (i + 1) << ": " 
                                      << solutions[i].toString() << "\n";
                        }
                    }
                }
            } catch (const std::exception& e) {
                std::cout << "  Error: " << e.what() << "\n";
            }
            
            std::cout << "\n";
        }
        
        // Mathematical sequence demonstrations
        std::cout << "Mathematical Sequences:\n";
        std::cout << "-----------------------\n";
        
        std::cout << "Fibonacci sequence (first 10 numbers):\n";
        for (int i = 0; i < 10; ++i) {
            try {
                auto solutions = interpreter.query("fibonacci(" + std::to_string(i) + ", X)");
                if (!solutions.empty() && solutions[0].bindings.find("X") != solutions[0].bindings.end()) {
                    std::cout << "F(" << i << ") = " << solutions[0].bindings["X"]->toString() << "\n";
                }
            } catch (const std::exception& e) {
                std::cout << "Error computing F(" << i << "): " << e.what() << "\n";
            }
        }
        
        std::cout << "\nFactorials:\n";
        for (int i = 0; i <= 7; ++i) {
            try {
                auto solutions = interpreter.query("factorial(" + std::to_string(i) + ", X)");
                if (!solutions.empty() && solutions[0].bindings.find("X") != solutions[0].bindings.end()) {
                    std::cout << i << "! = " << solutions[0].bindings["X"]->toString() << "\n";
                }
            } catch (const std::exception& e) {
                std::cout << "Error computing " << i << "!: " << e.what() << "\n";
            }
        }
        
        std::cout << "\nPrime number check (2-20):\n";
        for (int i = 2; i <= 20; ++i) {
            try {
                auto solutions = interpreter.query("is_prime(" + std::to_string(i) + ")");
                std::cout << i << " is " << (solutions.empty() ? "not prime" : "prime") << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error checking primality of " << i << ": " << e.what() << "\n";
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}