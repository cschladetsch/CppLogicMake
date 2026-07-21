// resolver.hpp
//
// Builds structured target data by running a fixed set of queries
// against prolog/targets.pl's schema, loaded together with a project
// file through PrologEngine.
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace logicmake {

// One compile_options(Target, CompilerId, Flags) fact. CompilerId is a
// literal CMAKE_CXX_COMPILER_ID value ('Clang'|'MSVC'|'GNU'|...); emitted
// as a generator expression so a single generated CMakeLists.txt is
// correct regardless of which compiler is chosen later at `cmake`
// configure time (see prolog/targets.pl for the full rationale).
struct CompileOptionGroup {
    std::string compilerId;
    std::string flags;
};

struct TargetInfo {
    std::string name;
    std::string kind;                  // lib | exe | interface
    std::string cxxStandard;           // "17"|"20"|"23"…; empty = file default
    std::vector<std::string> sources;
    std::vector<std::string> includes;
    std::vector<std::string> dependsPublic;
    std::vector<std::string> dependsPrivate;
    std::vector<std::string> dependsAll;
    std::vector<std::string> links;
    std::vector<std::string> defines;
    std::vector<CompileOptionGroup> compileOptions;
    std::string installDest;           // empty = no install(TARGETS ...) rule
    bool cyclic = false;
};

// A project-level submodule(Name, Url, Path) fact. Not target-scoped, so
// it lives outside TargetInfo — see Resolver::resolveSubmodules.
struct SubmoduleInfo {
    std::string name;
    std::string url;
    std::string path;
};

class Resolver {
public:
    // schemaPath: prolog/targets.pl
    // projectPath: the user's .lm file, e.g. examples/kai_workspace.lm
    Resolver(std::filesystem::path schemaPath, std::filesystem::path projectPath);

    [[nodiscard]] std::vector<TargetInfo> resolve() const;

    // Project-level submodule(Name, Url, Path) facts. Separate from
    // resolve() because these aren't targets — no sources/links/defines
    // to resolve, just a name/url/path triple for the driver to validate
    // against the filesystem before trusting any sources/2 fact that
    // reaches into Path.
    [[nodiscard]] std::vector<SubmoduleInfo> resolveSubmodules() const;

private:
    std::filesystem::path schemaPath_;
    std::filesystem::path projectPath_;
};

}  // namespace logicmake
