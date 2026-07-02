// test_resolver.cpp — exercises the real embedded CppProlog engine
// against prolog/targets.pl and examples/kai_workspace.pl. Requires
// CTest's WORKING_DIRECTORY to be the repo root (set in
// tests/CMakeLists.txt) since both paths are repo-relative.
#include <gtest/gtest.h>

#include <algorithm>

#include "resolver.hpp"

namespace {

logicmake::TargetInfo findTarget(const std::vector<logicmake::TargetInfo>& targets,
                                  const std::string& name) {
    auto it = std::find_if(targets.begin(), targets.end(),
                            [&](const auto& t) { return t.name == name; });
    if (it == targets.end()) {
        ADD_FAILURE() << "target not found: " << name;
        return {};
    }
    return *it;
}

bool contains(const std::vector<std::string>& haystack, const std::string& needle) {
    return std::find(haystack.begin(), haystack.end(), needle) != haystack.end();
}

}  // namespace

TEST(Resolver, ResolvesAllFiveTargets) {
    logicmake::Resolver resolver("prolog/targets.pl", "examples/kai_workspace.pl");
    const auto targets = resolver.resolve();
    EXPECT_EQ(targets.size(), 5u);
}

TEST(Resolver, TransitiveDependsAllIncludesPrivateDeps) {
    logicmake::Resolver resolver("prolog/targets.pl", "examples/kai_workspace.pl");
    const auto node = findTarget(resolver.resolve(), "kai_node");

    // kai_node -> kai_core -> {kai_language, enet, fmt(private)}
    EXPECT_TRUE(contains(node.dependsAll, "kai_core"));
    EXPECT_TRUE(contains(node.dependsAll, "kai_language"));
    EXPECT_TRUE(contains(node.dependsAll, "enet"));
    EXPECT_TRUE(contains(node.dependsAll, "fmt"));
}

TEST(Resolver, PlatformGuardedLinkResolvesForLinux) {
    logicmake::Resolver resolver("prolog/targets.pl", "examples/kai_workspace.pl");
    const auto node = findTarget(resolver.resolve(), "kai_node");

    // examples/kai_workspace.pl asserts platform(linux); enet's
    // link(enet, pthread) guard should be the only one to survive.
    EXPECT_TRUE(contains(node.links, "pthread"));
    EXPECT_FALSE(contains(node.links, "ws2_32"));
    EXPECT_FALSE(contains(node.links, "winmm"));
}

TEST(Resolver, UnassertedDebugGuardYieldsNoDefines) {
    logicmake::Resolver resolver("prolog/targets.pl", "examples/kai_workspace.pl");
    const auto core = findTarget(resolver.resolve(), "kai_core");

    // define(kai_core, 'KAI_DEBUG') :- debug. — debug is commented out
    // in the example, so this must resolve to zero defines.
    EXPECT_TRUE(core.defines.empty());
}

TEST(Resolver, SourcesAreRealGitTrackedFilesNotRawGlobs) {
    // examples/kai_workspace.pl declares sources(kai_core, "examples/
    // kai_workspace/CppKaiCore/src/*.cpp"), a pathspec, not a literal
    // filename. CMake does not glob-expand a wildcard passed to
    // add_library — this must come back as the real, git-tracked file
    // list, not the raw pathspec string, or the emitted CMakeLists.txt
    // fails to configure (see README's "Why this exists").
    logicmake::Resolver resolver("prolog/targets.pl", "examples/kai_workspace.pl");
    const auto core = findTarget(resolver.resolve(), "kai_core");

    EXPECT_EQ(core.sources.size(), 2u);
    EXPECT_TRUE(contains(core.sources,
                          "examples/kai_workspace/CppKaiCore/src/core.cpp"));
    EXPECT_TRUE(contains(core.sources,
                          "examples/kai_workspace/CppKaiCore/src/registry.cpp"));
    // No stray quote characters from the underlying String term's
    // toString() (see prolog_engine.hpp).
    for (const auto& s : core.sources) {
        EXPECT_EQ(s.find('"'), std::string::npos) << s;
    }
}

TEST(Resolver, NoCyclesInTheExampleWorkspace) {
    logicmake::Resolver resolver("prolog/targets.pl", "examples/kai_workspace.pl");
    for (const auto& t : resolver.resolve()) {
        EXPECT_FALSE(t.cyclic) << t.name;
    }
}

TEST(Resolver, ResolvesHelloWorldExample) {
    // The minimal single-exe example (examples/hello_world.pl): one exe
    // target whose "examples/hello_world/*.cpp" pathspec resolves to the
    // real git-tracked source, no deps, no links, no defines.
    logicmake::Resolver resolver("prolog/targets.pl", "examples/hello_world.pl");
    const auto targets = resolver.resolve();

    ASSERT_EQ(targets.size(), 1u);
    const auto hello = targets.front();
    EXPECT_EQ(hello.name, "hello_world");
    EXPECT_EQ(hello.kind, "exe");
    EXPECT_TRUE(contains(hello.sources, "examples/hello_world/main.cpp"));
    EXPECT_TRUE(hello.dependsAll.empty());
    EXPECT_TRUE(hello.links.empty());
    EXPECT_TRUE(hello.defines.empty());
    EXPECT_FALSE(hello.cyclic);
}

TEST(Resolver, DependsOnFindsEverythingThatWouldBreak) {
    // depends_on(fmt, T) — both kai_core (direct private dep) and
    // kai_node (transitive) should come back; this is the "what
    // breaks if I drop this dependency" query the README describes.
    logicmake::Resolver resolver("prolog/targets.pl", "examples/kai_workspace.pl");
    const auto targets = resolver.resolve();

    // Sanity check via depends_all rather than re-querying Prolog
    // directly: every target whose dependsAll contains "fmt".
    std::vector<std::string> affected;
    for (const auto& t : targets) {
        if (contains(t.dependsAll, "fmt")) affected.push_back(t.name);
    }
    EXPECT_TRUE(contains(affected, "kai_core"));
    EXPECT_TRUE(contains(affected, "kai_node"));
}
