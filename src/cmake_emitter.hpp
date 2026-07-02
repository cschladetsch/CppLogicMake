// cmake_emitter.hpp
#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "resolver.hpp"

namespace logicmake {

// Renders resolved targets as an ordinary, human-readable CMakeLists.txt.
// The output has no dependency on CppLogicMake or CppProlog at all — it
// is exactly what someone would have written by hand, just generated
// instead of maintained. gitStamp, if present, is embedded as a header
// comment (see git_integration.hpp) so a generated file carries which
// commit it was generated from.
//
// outputDir is the directory the generated CMakeLists.txt will be
// written to. Source and include paths arrive relative to the driver's
// working directory (the repo root, where `git ls-files` resolved
// them), but CMake resolves a target's relative paths against the
// directory containing the CMakeLists.txt — so unless that file lands
// at the working directory, the paths must be rebased to be relative to
// outputDir or configure fails to find the sources. An empty outputDir
// means "written at the working directory" and leaves paths untouched
// (the historical single-file-at-repo-root behaviour, and what the
// pure-string emitter tests rely on).
[[nodiscard]] std::string emitCMakeLists(
    const std::vector<TargetInfo>& targets,
    const std::optional<std::string>& gitStamp = std::nullopt,
    const std::filesystem::path& outputDir = {});

}  // namespace logicmake
