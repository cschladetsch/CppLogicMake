I got tired of writing CMakeLists.txt by hand, so I taught a Prolog engine to do it.

CMake's underlying model — a DAG of targets with public/private dependency propagation — is genuinely good. The *syntax* you write it in is not: stringly-typed booleans, quoting rules that change with context, `PUBLIC`/`PRIVATE`/`INTERFACE` as positional string arguments, and generator expressions invented because the host language couldn't express a conditional cleanly.

A build description isn't a script. It's a graph. So **CppLogicMake** lets you describe your build as plain facts:

```prolog
target(app, exe).
sources(app, "src/*.cpp").
depends(app, core).
link(core, ws2_32) :- platform(windows).
```

...and an embedded Prolog engine (CppProlog) resolves the graph — transitive dependencies, conditional links, cycle detection — then emits an ordinary, human-readable CMakeLists.txt. CMake still does what it's always done well: generating Ninja, Xcode, and MSVC projects from that file.

Why Prolog? Because the operations you actually want *are* logic queries:

• Transitive dependency closure — two clauses, not a hand-rolled traversal
• Conditional links and defines — guards, not a bolted-on generator-expression mini-language
• "What breaks if I drop this dependency?" — `depends_on(lib, T)`. Free, out of the same rules that resolve the graph.

A few things it grew recently:

→ `.lm` project files, resolved in parallel (one engine per file, no shared state)
→ git-backed source resolution via `git ls-files` instead of fragile globs — no more `add_library(foo src/*.cpp)` silently reading a wildcard as a literal filename
→ per-target C++ standard, so a C++17 target and a C++23 target can live in one project
→ a global `logimake` command, installed with one script
→ a cross-language demo (C++23 / C++17 / Python / Rust) that all agree on the same answer, wired into the test suite as an end-to-end check

It's C++23, small on purpose, and deliberately a thin, mechanical transpile layer — a callback to "C with Classes" compiled down by Cfront: keep the substrate, replace only the authoring syntax.

Code (and the reasoning behind the semantic boundary): https://github.com/cschladetsch/CppLogicMake

#cpp #cpp23 #cmake #prolog #buildsystems #softwareengineering
