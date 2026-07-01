// cmake_emitter.hpp
#pragma once

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
[[nodiscard]] std::string emitCMakeLists(
    const std::vector<TargetInfo>& targets,
    const std::optional<std::string>& gitStamp = std::nullopt);

}  // namespace logicmake
