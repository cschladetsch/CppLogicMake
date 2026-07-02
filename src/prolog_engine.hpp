// prolog_engine.hpp
//
// Thin wrapper around prolog::Interpreter (Ext/CppProlog/src/prolog),
// embedded directly and linked as a library — no subprocess, no
// stdin/stdout protocol to keep in sync with the interpreter's CLI.
//
// Two things about CppProlog's semantics that shaped this wrapper,
// confirmed against the actual source (Ext/CppProlog/src/prolog):
//
//   1. Interpreter::loadFile parses a file as a flat sequence of facts
//      and rules (Database::loadProgram just calls Parser::parseProgram
//      and asserts every clause). It does not execute ":- Goal."
//      directives — there is no special-casing for the RULE_OP token
//      when it appears with no head. Project .lm files therefore
//      contain only facts and rules, never directives; the driver
//      calls loadFile once per file instead.
//
//   2. Both '...' and "..." literals tokenize to Token::STRING, and
//      String::toString() wraps the value in escaped double quotes
//      (see Ext/CppProlog/src/prolog/term.h). A bound variable whose
//      value came from either quoting style therefore comes back from
//      query() with a wrapping pair of quotes still attached — stripped
//      once, centrally, in Resolver rather than at every call site.
#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace prolog {
class Interpreter;
}

namespace logicmake {

class PrologEngine {
public:
    PrologEngine();
    ~PrologEngine();

    PrologEngine(const PrologEngine&) = delete;
    PrologEngine& operator=(const PrologEngine&) = delete;

    void loadFile(const std::filesystem::path& path);

    // Runs `goal` and returns one row per solution, with one string per
    // requested variable name, in the order given. A solution missing
    // one of the requested variables is skipped rather than throwing —
    // that shouldn't happen for well-formed schema queries, but a
    // partial row would otherwise silently misalign with `vars`.
    [[nodiscard]] std::vector<std::vector<std::string>> query(
        const std::string& goal, const std::vector<std::string>& vars) const;

private:
    std::unique_ptr<prolog::Interpreter> interpreter_;
};

// Forces CppProlog's builtin-predicate table to finish initializing on
// the calling thread. Call this once, single-threaded, before
// constructing any PrologEngine concurrently from multiple threads —
// see prolog_engine.cpp for the data race this avoids (confirmed under
// ThreadSanitizer against Ext/CppProlog directly: two Interpreters
// constructed on different threads for the first time race on
// BuiltinPredicates::builtins_, an unsynchronized static
// std::unordered_map populated via an unguarded check-then-act).
void warmUpPrologRuntime();

}  // namespace logicmake
