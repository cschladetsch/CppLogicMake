// prolog_engine.cpp
#include "prolog_engine.hpp"

#include "prolog/interpreter.h"

namespace logicmake {

PrologEngine::PrologEngine()
    : interpreter_(std::make_unique<prolog::Interpreter>(/*interactive=*/false)) {}

// Defined here rather than defaulted in the header: prolog::Interpreter
// is only forward-declared in prolog_engine.hpp, so ~unique_ptr needs
// the complete type available, which this translation unit has via
// interpreter.h.
PrologEngine::~PrologEngine() = default;

void PrologEngine::loadFile(const std::filesystem::path& path) {
    interpreter_->loadFile(path.string());
}

std::vector<std::vector<std::string>> PrologEngine::query(
    const std::string& goal, const std::vector<std::string>& vars) const {
    const auto solutions = interpreter_->query(goal);

    std::vector<std::vector<std::string>> rows;
    rows.reserve(solutions.size());

    for (const auto& solution : solutions) {
        std::vector<std::string> row;
        row.reserve(vars.size());
        bool complete = true;

        for (const auto& var : vars) {
            const auto it = solution.bindings.find(var);
            if (it == solution.bindings.end()) {
                complete = false;
                break;
            }
            row.push_back(it->second->toString());
        }

        if (complete) {
            rows.push_back(std::move(row));
        }
    }

    return rows;
}

// external/CppProlog/src/prolog/builtin_predicates.cpp:
//
//   void BuiltinPredicates::registerBuiltins() {
//       if (!builtins_.empty()) return;   // <-- unsynchronized read
//       builtins_[...] = ...;              // <-- unsynchronized write
//       ...
//   }
//
// builtins_ is a plain static class member, not a function-local
// static (which would get thread-safe magic-statics initialization).
// Every Interpreter construction calls this. Two Interpreters
// constructed on different threads before the map is populated race:
// confirmed with `clang++ -fsanitize=thread` against a small program
// spawning eight threads that each construct an Interpreter with no
// synchronization — ThreadSanitizer reported a data race at exactly
// this line on every run. Constructing one Interpreter here, fully,
// before any concurrent construction happens, populates the map once;
// after that every access is a read of a map nobody mutates again,
// which is safe under the memory model because thread creation is a
// happens-before edge for everything sequenced before it. Verified:
// the same eight-thread program is race-free under TSan once this
// warm-up runs first.
//
// This lives in CppLogicMake rather than as a patch to external/CppProlog
// because the submodule is pinned as an external dependency, not
// something this repo edits in place.
void warmUpPrologRuntime() {
    PrologEngine warmup;
    (void)warmup;
}

}  // namespace logicmake
