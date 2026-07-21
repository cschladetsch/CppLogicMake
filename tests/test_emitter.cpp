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

TEST(CMakeEmitter, PerTargetCxxStandardOverridesFileDefault) {
    // A target carrying an explicit cxx_standard must get a
    // set_target_properties override on top of the file-level
    // CMAKE_CXX_STANDARD; a target without one must not.
    TargetInfo modern;
    modern.name = "fib_cpp23";
    modern.kind = "exe";
    modern.sources = {"cpp23/main.cpp"};
    modern.cxxStandard = "23";

    TargetInfo legacy;
    legacy.name = "fib_cpp17";
    legacy.kind = "exe";
    legacy.sources = {"cpp17/main.cpp"};
    legacy.cxxStandard = "17";

    TargetInfo plain;
    plain.name = "plain";
    plain.kind = "exe";
    plain.sources = {"plain/main.cpp"};

    const auto out = logicmake::emitCMakeLists({modern, legacy, plain});
    EXPECT_NE(out.find("set_target_properties(fib_cpp23 PROPERTIES "
                       "CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)"),
              std::string::npos);
    EXPECT_NE(out.find("set_target_properties(fib_cpp17 PROPERTIES "
                       "CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)"),
              std::string::npos);
    EXPECT_EQ(out.find("set_target_properties(plain"), std::string::npos);
}

TEST(CMakeEmitter, InterfaceTargetIgnoresCxxStandard) {
    // An INTERFACE target has nothing to compile, so a stray
    // cxx_standard on it must not produce a set_target_properties line.
    TargetInfo iface;
    iface.name = "headers";
    iface.kind = "interface";
    iface.cxxStandard = "23";

    const auto out = logicmake::emitCMakeLists({iface});
    EXPECT_EQ(out.find("set_target_properties(headers"), std::string::npos);
}

TEST(CMakeEmitter, PathRebasingIsCleanWhenSourceIsUnderOutputDir) {
    // When a source already lives under the output directory the rebased
    // path must be a plain descent with no leading "../" — this is the
    // other side of the lexical relative computation and would regress
    // silently if rebasing were done by naively counting separators.
    TargetInfo exe;
    exe.name = "gen";
    exe.kind = "exe";
    exe.sources = {"build/gen/generated/main.cpp"};

    const auto out = logicmake::emitCMakeLists({exe}, std::nullopt, "build/gen");
    EXPECT_NE(out.find("add_executable(gen generated/main.cpp)"), std::string::npos);
}

TEST(CMakeEmitter, MixedCAndCxxSourcesEmitBothLanguages) {
    // A target whose sources include a plain .c file (e.g. a vendored C
    // library like GLFW, added via sources/2 facts rather than
    // find_package/FetchContent) needs `project(... LANGUAGES C CXX)` or
    // CMake never configures a C compiler and configure fails outright.
    TargetInfo glfw;
    glfw.name = "glfw";
    glfw.kind = "lib";
    glfw.sources = {"Ext/glfw/src/context.c", "Ext/glfw/src/win32_init.c"};

    const auto out = logicmake::emitCMakeLists({glfw});
    EXPECT_NE(out.find("project(generated LANGUAGES C CXX)"), std::string::npos);
}

TEST(CMakeEmitter, PureCxxProjectStillEmitsCxxOnlyLanguage) {
    // Backward compatibility: a project with no .c sources anywhere must
    // keep emitting the original CXX-only language line unchanged.
    TargetInfo hello;
    hello.name = "hello";
    hello.kind = "exe";
    hello.sources = {"src/main.cpp"};

    const auto out = logicmake::emitCMakeLists({hello});
    EXPECT_NE(out.find("project(generated LANGUAGES CXX)"), std::string::npos);
    EXPECT_EQ(out.find("LANGUAGES C CXX"), std::string::npos);
}

TEST(CMakeEmitter, CompileOptionsEmitPerCompilerGeneratorExpressions) {
    // One compile_options/3 fact per compiler -> one quoted generator
    // expression each, so the same generated file is correct regardless
    // of which compiler CMAKE_CXX_COMPILER_ID resolves to later.
    TargetInfo exe;
    exe.name = "procmon";
    exe.kind = "exe";
    exe.sources = {"src/main.cpp"};
    exe.compileOptions = {
        {"Clang", "-Wall -Wextra -Wpedantic"},
        {"MSVC", "/W4 /utf-8"},
    };

    const auto out = logicmake::emitCMakeLists({exe});
    EXPECT_NE(out.find("target_compile_options(procmon PRIVATE "
                       "\"$<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>\" "
                       "\"$<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>\")"),
              std::string::npos);
}

TEST(CMakeEmitter, NoCompileOptionsFactsEmitNothing) {
    TargetInfo exe;
    exe.name = "plain";
    exe.kind = "exe";
    exe.sources = {"src/main.cpp"};

    const auto out = logicmake::emitCMakeLists({exe});
    EXPECT_EQ(out.find("target_compile_options"), std::string::npos);
}

TEST(CMakeEmitter, InterfaceTargetIgnoresCompileOptions) {
    // Nothing of an INTERFACE target's own compiles, so a stray
    // compile_options/3 fact on one must not produce a
    // target_compile_options call.
    TargetInfo iface;
    iface.name = "headers";
    iface.kind = "interface";
    iface.compileOptions = {{"Clang", "-Wall"}};

    const auto out = logicmake::emitCMakeLists({iface});
    EXPECT_EQ(out.find("target_compile_options"), std::string::npos);
}

TEST(CMakeEmitter, InstallDestEmitsInstallRule) {
    TargetInfo exe;
    exe.name = "procmon";
    exe.kind = "exe";
    exe.sources = {"src/main.cpp"};
    exe.installDest = "bin";

    const auto out = logicmake::emitCMakeLists({exe});
    EXPECT_NE(out.find("install(TARGETS procmon RUNTIME DESTINATION bin)"),
              std::string::npos);
}

TEST(CMakeEmitter, NoInstallFactEmitsNoInstallRule) {
    TargetInfo exe;
    exe.name = "plain";
    exe.kind = "exe";
    exe.sources = {"src/main.cpp"};

    const auto out = logicmake::emitCMakeLists({exe});
    EXPECT_EQ(out.find("install("), std::string::npos);
}

TEST(CMakeEmitter, InterfaceTargetIgnoresInstallDest) {
    TargetInfo iface;
    iface.name = "headers";
    iface.kind = "interface";
    iface.installDest = "bin";

    const auto out = logicmake::emitCMakeLists({iface});
    EXPECT_EQ(out.find("install("), std::string::npos);
}

TEST(CMakeEmitter, CDetectionIsCaseInsensitiveAndIgnoresLookalikeExtensions) {
    // ".C" (uppercase, a valid C source extension on case-sensitive
    // filesystems) must still trigger C-language mode, while extensions
    // that merely contain "c" (".cpp", ".cc", ".cxx") must not.
    TargetInfo upper;
    upper.name = "legacy";
    upper.kind = "lib";
    upper.sources = {"legacy/old.C"};

    const auto out = logicmake::emitCMakeLists({upper});
    EXPECT_NE(out.find("project(generated LANGUAGES C CXX)"), std::string::npos);

    TargetInfo cxxVariants;
    cxxVariants.name = "variants";
    cxxVariants.kind = "lib";
    cxxVariants.sources = {"src/a.cpp", "src/b.cc", "src/c.cxx"};

    const auto out2 = logicmake::emitCMakeLists({cxxVariants});
    EXPECT_EQ(out2.find("LANGUAGES C CXX"), std::string::npos);
}
