// cmake_emitter.cpp
#include "cmake_emitter.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <sstream>

namespace logicmake {

namespace {

// project(generated LANGUAGES CXX) alone leaves CMake without a configured
// C compiler, which breaks configure for any target whose sources include
// plain .c files (e.g. a vendored C library like GLFW, added as
// sources/2 facts rather than pulled in via find_package/FetchContent -
// see README "Non-goals"). Scanning resolved sources for a .c extension
// and switching to LANGUAGES C CXX when found keeps existing pure-C++
// projects emitting exactly what they did before (no .c sources = no
// behavior change) while making mixed C/C++ targets configure correctly.
bool anyTargetHasCSources(const std::vector<TargetInfo>& targets) {
    for (const auto& t : targets) {
        for (const auto& src : t.sources) {
            std::string ext = std::filesystem::path(src).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (ext == ".c") {
                return true;
            }
        }
    }
    return false;
}

// Rewrites a path emitted by the resolver (relative to the driver's
// working directory) so it is instead relative to outputDir, the
// directory the generated CMakeLists.txt lives in. This is purely
// lexical — filesystem::absolute only prepends current_path() and
// lexically_relative never touches disk — so it is safe for paths that
// don't exist yet and produces forward slashes regardless of platform.
// An empty outputDir means the file is written at the working directory
// (repo root), where the resolver's paths are already correct, so the
// original string is returned verbatim.
std::string rebasePath(const std::string& p,
                       const std::filesystem::path& outputDir) {
    if (outputDir.empty()) {
        return p;
    }
    const auto base = std::filesystem::current_path();
    std::filesystem::path abs(p);
    if (!abs.is_absolute()) {
        abs = base / abs;
    }
    std::filesystem::path absDir = outputDir;
    if (!absDir.is_absolute()) {
        absDir = base / absDir;
    }
    return abs.lexically_relative(absDir).generic_string();
}

// A "lib" target with zero resolved sources isn't valid CMake either
// way — add_library(name) with no sources fails to configure exactly
// like the pathspec-glob bug this tool exists to avoid. In this
// schema that shape means a target standing in for something outside
// the workspace (a system library, a future find_package call —
// find_package interop is an explicit non-goal for now, see README).
// Emitting it as INTERFACE is the only shape that's both valid CMake
// and doesn't invent build steps nobody asked for; the comment makes
// the substitution visible rather than silent.
bool emitAsInterface(const TargetInfo& t) {
    return t.kind == "interface" || (t.kind == "lib" && t.sources.empty());
}

void emitTarget(std::ostringstream& out, const TargetInfo& t,
                const std::filesystem::path& outputDir) {
    out << "# --- " << t.name << " ---\n";

    const bool asInterface = emitAsInterface(t);
    if (asInterface && t.kind != "interface") {
        out << "# NOTE: '" << t.name
            << "' has no resolved sources; emitted as INTERFACE rather "
               "than a real compiled library.\n";
    }

    if (asInterface) {
        out << "add_library(" << t.name << " INTERFACE)\n";
        for (const auto& inc : t.includes) {
            out << "target_include_directories(" << t.name
                << " INTERFACE " << rebasePath(inc, outputDir) << ")\n";
        }
    } else {
        out << (t.kind == "exe" ? "add_executable" : "add_library") << "("
            << t.name;
        for (const auto& src : t.sources) {
            out << " " << rebasePath(src, outputDir);
        }
        out << ")\n";

        for (const auto& inc : t.includes) {
            out << "target_include_directories(" << t.name
                << " PUBLIC " << rebasePath(inc, outputDir) << ")\n";
        }
    }

    // A per-target standard overrides the file-level CMAKE_CXX_STANDARD
    // default. Only meaningful for something that actually compiles, so
    // it is skipped for INTERFACE targets (which have no sources).
    if (!asInterface && !t.cxxStandard.empty()) {
        out << "set_target_properties(" << t.name
            << " PROPERTIES CXX_STANDARD " << t.cxxStandard
            << " CXX_STANDARD_REQUIRED ON)\n";
    }

    auto emitLinkGroup = [&](const char* scope, const std::vector<std::string>& items) {
        if (items.empty()) {
            return;
        }
        out << "target_link_libraries(" << t.name << scope;
        for (const auto& item : items) {
            out << " " << item;
        }
        out << ")\n";
    };

    if (asInterface) {
        std::vector<std::string> interfaceDeps = t.dependsPublic;
        interfaceDeps.insert(interfaceDeps.end(), t.dependsPrivate.begin(),
                             t.dependsPrivate.end());
        interfaceDeps.insert(interfaceDeps.end(), t.links.begin(), t.links.end());
        emitLinkGroup(" INTERFACE", interfaceDeps);
    } else {
        const char* depScope = t.kind == "exe" ? " PRIVATE" : " PUBLIC";
        emitLinkGroup(depScope, t.dependsPublic);
        emitLinkGroup(" PRIVATE", t.dependsPrivate);
        emitLinkGroup(" PRIVATE", t.links);
    }

    if (!asInterface && !t.dependsAll.empty()) {
        out << "# NOTE: resolved closure for " << t.name
            << " is tracked in the semantic model, not flattened into CMake.\n";
    }

    if (!t.defines.empty()) {
        const char* defineScope = asInterface ? " INTERFACE"
                                 : (t.kind == "exe" ? " PRIVATE" : " PUBLIC");
        out << "target_compile_definitions(" << t.name << defineScope;
        for (const auto& def : t.defines) {
            out << " " << def;
        }
        out << ")\n";
    }

    out << "\n";
}

}  // namespace

std::string emitCMakeLists(const std::vector<TargetInfo>& targets,
                            const std::optional<std::string>& gitStamp,
                            const std::filesystem::path& outputDir) {
    std::ostringstream out;
    out << "# Generated by CppLogicMake — do not edit by hand.\n";
    out << "# Source of truth is the .lm project file this was built from.\n";
    if (gitStamp) {
        out << "# Generated from commit " << *gitStamp << "\n";
    }
    out << "cmake_minimum_required(VERSION 3.25)\n";
    out << "project(generated LANGUAGES "
        << (anyTargetHasCSources(targets) ? "C CXX" : "CXX") << ")\n\n";
    out << "set(CMAKE_CXX_STANDARD 23)\n";
    out << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";

    for (const auto& t : targets) {
        if (t.cyclic) {
            out << "# WARNING: '" << t.name
                << "' participates in a dependency cycle. "
                << "Resolve before building.\n";
        }
        emitTarget(out, t, outputDir);
    }

    return out.str();
}

}  // namespace logicmake
