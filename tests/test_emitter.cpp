// test_emitter.cpp — the emitter is pure (TargetInfo in, string out)
// and needs no CppProlog interpreter present, unlike test_resolver.cpp.
#include <gtest/gtest.h>

#include "cmake_emitter.hpp"
#include "resolver.hpp"

namespace {

using logicmake::TargetInfo;

std::vector<TargetInfo> makeSampleTargets() {
    TargetInfo iface;
    iface.name = "kai_language";
    iface.kind = "interface";
    iface.includes = {"include/"};

    TargetInfo core;
    core.name = "kai_core";
    core.kind = "lib";
    core.sources = {"src/*.cpp"};
    core.includes = {"include/"};
    core.dependsPublic = {"kai_language", "enet"};
    core.dependsPrivate = {"fmt"};
    core.dependsAll = {"kai_language", "enet", "fmt"};
    core.links = {"pthread"};
    core.defines = {"KAI_DEBUG"};

    TargetInfo node;
    node.name = "kai_node";
    node.kind = "exe";
    node.sources = {"node/*.cpp"};
    node.dependsPublic = {"kai_core"};
    node.dependsAll = {"kai_core"};
    node.cyclic = true;

    return {iface, core, node};
}

}  // namespace

TEST(CMakeEmitter, InterfaceTargetEmitsInterfaceKeyword) {
    const auto out = logicmake::emitCMakeLists(makeSampleTargets());
    EXPECT_NE(out.find("add_library(kai_language INTERFACE)"), std::string::npos);
}

TEST(CMakeEmitter, LibTargetEmitsSources) {
    const auto out = logicmake::emitCMakeLists(makeSampleTargets());
    EXPECT_NE(out.find("add_library(kai_core src/*.cpp)"), std::string::npos);
}

TEST(CMakeEmitter, PublicAndPrivateDepsArePreserved) {
    const auto out = logicmake::emitCMakeLists(makeSampleTargets());
    EXPECT_NE(out.find("target_link_libraries(kai_core PUBLIC kai_language enet)"),
              std::string::npos);
    EXPECT_NE(out.find("target_link_libraries(kai_core PRIVATE fmt)"),
              std::string::npos);
}

TEST(CMakeEmitter, ResolvedLinksAreEmitted) {
    const auto out = logicmake::emitCMakeLists(makeSampleTargets());
    EXPECT_NE(out.find("target_link_libraries(kai_core PRIVATE pthread)"),
              std::string::npos);
}

TEST(CMakeEmitter, ResolvedDefinesAreEmitted) {
    const auto out = logicmake::emitCMakeLists(makeSampleTargets());
    EXPECT_NE(out.find("target_compile_definitions(kai_core PUBLIC KAI_DEBUG)"),
              std::string::npos);
}

TEST(CMakeEmitter, ExeTargetUsesAddExecutable) {
    const auto out = logicmake::emitCMakeLists(makeSampleTargets());
    EXPECT_NE(out.find("add_executable(kai_node node/*.cpp)"), std::string::npos);
}

TEST(CMakeEmitter, ExeDepsStayPrivate) {
    const auto out = logicmake::emitCMakeLists(makeSampleTargets());
    EXPECT_NE(out.find("target_link_libraries(kai_node PRIVATE kai_core)"),
              std::string::npos);
}

TEST(CMakeEmitter, CyclicTargetGetsWarningComment) {
    const auto out = logicmake::emitCMakeLists(makeSampleTargets());
    EXPECT_NE(out.find("WARNING: 'kai_node' participates in a dependency cycle"),
              std::string::npos);
}

TEST(CMakeEmitter, SourcelessLibIsEmittedAsInterfaceNotBrokenAddLibrary) {
    // add_library(name) with zero sources fails to configure in real
    // CMake — same failure class as the unresolved-glob bug this tool
    // exists to avoid. A "lib" target with no resolved sources (e.g.
    // standing in for something outside the workspace) must come out
    // as INTERFACE, with a comment marking the substitution, not as a
    // bare add_library() call.
    TargetInfo external;
    external.name = "enet";
    external.kind = "lib";
    external.links = {"pthread"};

    const auto out = logicmake::emitCMakeLists({external});
    EXPECT_NE(out.find("add_library(enet INTERFACE)"), std::string::npos);
    EXPECT_NE(out.find("target_link_libraries(enet INTERFACE pthread)"),
              std::string::npos);
    EXPECT_NE(out.find("NOTE: 'enet' has no resolved sources"), std::string::npos);
}

TEST(CMakeEmitter, PathsAreRebasedRelativeToOutputDirectory) {
    // Source/include paths arrive relative to the repo root (where `git
    // ls-files` ran), but CMake resolves a target's relative paths
    // against the directory holding the CMakeLists.txt. If that file is
    // written to a subdirectory the paths must be rebased, or
    // `cmake -S <subdir>` fails to find the sources. Rebasing is purely
    // lexical against a common base, so the expected "../../" result is
    // independent of where the repo actually lives on disk.
    TargetInfo exe;
    exe.name = "hello_world";
    exe.kind = "exe";
    exe.sources = {"examples/hello_world/main.cpp"};
    exe.includes = {"examples/hello_world/include"};

    // Empty output dir (the default) = written at the repo root: paths
    // are already correct and must be left untouched.
    const auto rooted = logicmake::emitCMakeLists({exe});
    EXPECT_NE(rooted.find("add_executable(hello_world examples/hello_world/main.cpp)"),
              std::string::npos);

    // Written two levels down: paths must be rebased with "../../".
    const auto nested =
        logicmake::emitCMakeLists({exe}, std::nullopt, "build/hello_world");
    EXPECT_NE(
        nested.find("add_executable(hello_world ../../examples/hello_world/main.cpp)"),
        std::string::npos);
    EXPECT_NE(nested.find("target_include_directories(hello_world PUBLIC "
                          "../../examples/hello_world/include)"),
              std::string::npos);
}
