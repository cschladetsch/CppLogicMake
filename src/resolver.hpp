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
    bool cyclic = false;
};

class Resolver {
public:
    // schemaPath: prolog/targets.pl
    // projectPath: the user's .lm file, e.g. examples/kai_workspace.lm
    Resolver(std::filesystem::path schemaPath, std::filesystem::path projectPath);

    [[nodiscard]] std::vector<TargetInfo> resolve() const;

private:
    std::filesystem::path schemaPath_;
    std::filesystem::path projectPath_;
};

}  // namespace logicmake
