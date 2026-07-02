# CppLogicMake Hardening Plan

## Goal
Move CppLogicMake from "proven on one synthetic 5-target example" to "safe to point at a real, messy production repo and safe for someone other than the author to adopt." Ordered by what actually blocks adoption, not by difficulty.

## Phase 0 — Prove it on a real repo (do this before anything else)
The single highest-value next step. Everything else is polish until this happens.

- Point the driver at an actual multi-target CppLogicMake-hosted repo you own (CppKAI is the obvious candidate: it already has real submodule splits, real transitive deps, real platform guards).
- Do not hand-simplify the project to fit the current schema. Let it fail. Every failure is a schema gap or emitter bug worth knowing about now, not after someone else hits it.
- Track every gap found as an issue before fixing it, so the schema's actual coverage boundary is documented, not just implied by "non-goals."

## Phase 1 — Close the known schema gaps
These are the ones the README already admits to (good — means they're known, not hidden):

- `find_package` / imported targets. Even a narrow version — `find_package(Name)` as a fact, emitting a plain passthrough — covers a large fraction of real projects that currently can't use this tool at all.
- Install rules (`install(TARGETS ...)`). Needed for anything meant to be packaged or consumed downstream.
- `FetchContent` / external project fetch, at least as an escape-hatch passthrough fact rather than full modeling.
- C++ standard / language standard per target (`cxx_standard(Name, 23)` or similar). Currently absent from the schema entirely — worth confirming this is deliberate (defaulted elsewhere) rather than an oversight.
- An explicit **escape hatch**: a fact type that emits raw, unvalidated CMake text verbatim for the "genuinely irregular 20%" the README concedes exists. Without this, any project with one weird platform quirk can't adopt the tool at all. With it, adoption is incremental — most targets go through Prolog facts, the one ugly target keeps its handwritten CMake fragment.

## Phase 2 — Error surface and diagnostics
This is what determines whether someone other than you can debug a failure.

- Every hard error (unresolvable pathspec, cyclic dependency, non-git directory) needs to name the offending target/fact and the source `.pl` file/line, not just fail. Prolog resolution failures are notoriously opaque to newcomers — a bare unification failure with no context will kill adoption faster than any missing feature.
- Add a `--dry-run` / `--explain` mode that prints the resolved graph (targets, deps, links, defines) without emitting CMake — lets someone sanity-check their `.pl` file before it hits the CMake configure step.
- Cycle detection already exists (`cyclic/1`) — surface *which* cycle, not just that one exists, when it fires as a hard error.

## Phase 3 — Fuzz and adversarial input testing
16 GTests covering the happy path and a couple of known bug regressions is good but is not adversarial coverage.

- Malformed `.pl` files: missing required facts, wrong arity, `target/2` referencing a `Dep` that's never itself declared as `target/2`.
- Pathological pathspecs: patterns matching thousands of files, patterns matching zero files inside a valid git repo vs. outside one, patterns with git's `magic` exclusion syntax.
- Cycles at depth >2, diamond dependencies with conflicting guards (e.g. a lib both publicly and privately depended on through different paths).
- Concurrent driver invocations against overlapping project files (beyond the existing independent-project threading test).

## Phase 4 — Performance at real scale
Current numbers (~6ms for 5 targets) don't tell you anything about a 200-target repo.

- Benchmark against a generated synthetic project at 50 / 200 / 1000 targets, since the git-subprocess-per-pathspec cost is explicitly called out as the dominant cost and is linear in target count, not project size.
- If subprocess overhead becomes the bottleneck at scale, consider batching `git ls-files` calls (single invocation across all pathspecs in a project file) rather than one process per `sources/2` fact.

## Phase 5 — Adoption ergonomics (do this last, only once 0–4 are solid)
- A `logicmake init` that scaffolds a minimal `.pl` file from an existing handwritten `CMakeLists.txt` (even a naive best-effort reverse-mapping) lowers the barrier to trying it on an existing project enormously.
- Editor support: even basic syntax highlighting for the Prolog fact schema, or a JSON schema-style linter, matters more for adoption than any resolver feature.
- A CONTRIBUTING.md documenting the loading model, the two-pass `loadFile` behavior, and the "why not `:- consult`" decision — this is currently only discoverable by reading `src/prolog_engine.cpp`, which is a real barrier for any outside contributor.

## Explicit non-goals to keep saying no to
Keep restating these publicly, since they're doing real work protecting scope:
- Not a CMake generator-backend replacement.
- Not a general logic-programming platform.
- Not a build-system-agnostic tool — it's a CMake authoring layer, full stop. Resist pressure to add a non-CMake emitter target; that's a different, much bigger project.
