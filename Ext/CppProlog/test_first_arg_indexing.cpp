#include "src/prolog/database.h"
#include "src/prolog/term.h"
#include "src/prolog/clause.h"
#include <iostream>
#include <chrono>

using namespace prolog;

int main() {
    Database db;
    
    // Add test facts with different first arguments
    std::cout << "Adding test facts...\n";
    db.addFact(makeCompound("likes", {makeAtom("mary"), makeAtom("food")}));
    db.addFact(makeCompound("likes", {makeAtom("john"), makeAtom("wine")}));
    db.addFact(makeCompound("likes", {makeAtom("mary"), makeAtom("wine")}));
    db.addFact(makeCompound("likes", {makeAtom("bob"), makeAtom("beer")}));
    db.addFact(makeCompound("likes", {makeAtom("john"), makeAtom("beer")}));
    
    std::cout << "Database size: " << db.size() << " clauses\n";
    
    // Test first argument indexing
    std::cout << "\nTesting first argument indexing:\n";
    
    auto mary_clauses = db.findClausesWithFirstArg("likes", 2, makeAtom("mary"));
    std::cout << "Clauses with first argument 'mary': " << mary_clauses.size() << "\n";
    for (const auto& clause : mary_clauses) {
        std::cout << "  " << clause->toString() << "\n";
    }
    
    auto john_clauses = db.findClausesWithFirstArg("likes", 2, makeAtom("john"));
    std::cout << "Clauses with first argument 'john': " << john_clauses.size() << "\n";
    for (const auto& clause : john_clauses) {
        std::cout << "  " << clause->toString() << "\n";
    }
    
    auto bob_clauses = db.findClausesWithFirstArg("likes", 2, makeAtom("bob"));
    std::cout << "Clauses with first argument 'bob': " << bob_clauses.size() << "\n";
    for (const auto& clause : bob_clauses) {
        std::cout << "  " << clause->toString() << "\n";
    }
    
    // Test with non-existent first argument
    auto alice_clauses = db.findClausesWithFirstArg("likes", 2, makeAtom("alice"));
    std::cout << "Clauses with first argument 'alice': " << alice_clauses.size() << " (should be 0)\n";
    
    std::cout << "\nFirst argument indexing test completed successfully!\n";
    return 0;
}