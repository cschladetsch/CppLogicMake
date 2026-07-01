// resolver.cpp
#include "resolver.hpp"

#include <unordered_map>

#include "git_integration.hpp"
#include "prolog_engine.hpp"

namespace logicmake {

namespace {

// Both '...' and "..." literals come back from PrologEngine::query with
// a wrapping pair of escaped double quotes (see prolog_engine.hpp for
// why). Bare atoms — target names, kinds, link names — never have this,
// so stripping unconditionally is safe: it only ever removes a matching
// leading and trailing '"'.
std::string stripQuotes(std::string value) {
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

}  // namespace

Resolver::Resolver(std::filesystem::path schemaPath,
                    std::filesystem::path projectPath)
    : schemaPath_(std::move(schemaPath)), projectPath_(std::move(projectPath)) {}

std::vector<TargetInfo> Resolver::resolve() const {
    PrologEngine engine;
    engine.loadFile(schemaPath_);
    engine.loadFile(projectPath_);

    std::unordered_map<std::string, TargetInfo> byName;
    std::vector<std::string> order;

    auto ensure = [&](const std::string& name) -> TargetInfo& {
        auto it = byName.find(name);
        if (it == byName.end()) {
            order.push_back(name);
            auto [it2, ok] = byName.emplace(name, TargetInfo{.name = name});
            (void)ok;
            return it2->second;
        }
        return it->second;
    };

    for (const auto& row : engine.query("target(T,K)", {"T", "K"})) {
        ensure(row[0]).kind = row[1];
    }
    for (const auto& row : engine.query("sources(T,S)", {"T", "S"})) {
        const auto pathspec = stripQuotes(row[1]);
        const auto files = resolveGitSources(pathspec);
        auto& target = ensure(row[0]);
        target.sources.insert(target.sources.end(), files.begin(), files.end());
    }
    for (const auto& row : engine.query("include(T,I)", {"T", "I"})) {
        ensure(row[0]).includes.push_back(stripQuotes(row[1]));
    }
    for (const auto& row : engine.query("depends(T,D)", {"T", "D"})) {
        ensure(row[0]).dependsPublic.push_back(row[1]);
    }
    for (const auto& row : engine.query("depends(T,D,private)", {"T", "D"})) {
        ensure(row[0]).dependsPrivate.push_back(row[1]);
    }
    for (const auto& row : engine.query("depends_all(T,D)", {"T", "D"})) {
        ensure(row[0]).dependsAll.push_back(row[1]);
    }
    for (const auto& row : engine.query("resolved_link(T,L)", {"T", "L"})) {
        ensure(row[0]).links.push_back(stripQuotes(row[1]));
    }
    for (const auto& row : engine.query("resolved_define(T,M)", {"T", "M"})) {
        ensure(row[0]).defines.push_back(stripQuotes(row[1]));
    }
    for (const auto& row : engine.query("cyclic(T)", {"T"})) {
        ensure(row[0]).cyclic = true;
    }

    std::vector<TargetInfo> result;
    result.reserve(order.size());
    for (const auto& name : order) {
        result.push_back(byName.at(name));
    }
    return result;
}

}  // namespace logicmake
