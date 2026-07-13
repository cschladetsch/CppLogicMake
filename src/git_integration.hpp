// git_integration.hpp
//
// Two git-backed features, both driven by shelling out to `git`
// rather than linking libgit2 — keeps the dependency footprint at
// zero for something this small, consistent with the rest of the
// tool's approach to its dependencies.
#pragma once

#include <optional>
#include <string>
#include <vector>

namespace logicmake {

// Resolves a git pathspec (e.g. "src/*.cpp") to the exact list of
// currently tracked files matching it, via
// `git ls-files --recurse-submodules -- <pathspec>`. The
// --recurse-submodules flag means a pathspec reaching into a vendored
// git submodule (e.g. "Ext/glfw/src/*.c") resolves the files inside it
// too, not just the submodule's own gitlink entry — required for a
// submodule's sources to be usable as sources/2 facts at all.
//
// This exists because CMake does not glob-expand a wildcard passed
// directly to add_library/add_executable — `add_library(foo
// src/*.cpp)` is CMake taking "src/*.cpp" as a literal, single,
// nonexistent filename, not a pattern. Emitting a pathspec string into
// generated CMake output produces a CMakeLists.txt that fails to
// configure (confirmed: see repo history / README). Resolving to
// literal filenames at generation time, once, via git's own pathspec
// matching, is both the fix and — because git pathspecs are already
// tracked-files-only, .gitignore-aware, and support exclusions and
// magic patterns — a strictly more expressive source of truth than a
// shell glob would have been anyway.
//
// Throws std::runtime_error if git isn't on PATH, the current
// directory isn't inside a git repository, or the pathspec matches no
// tracked files. All three are treated as hard errors rather than an
// empty target, since a target with silently zero sources is a worse
// failure mode than a loud one.
[[nodiscard]] std::vector<std::string> resolveGitSources(const std::string& pathspec);

// A short provenance stamp for the current git state: short commit
// hash, with "-dirty" appended if the working tree has uncommitted
// changes. std::nullopt if the current directory isn't inside a git
// repository — this is decoration for generated output, never
// load-bearing, so its absence doesn't abort generation the way a
// failed resolveGitSources call does.
[[nodiscard]] std::optional<std::string> gitProvenanceStamp();

}  // namespace logicmake
