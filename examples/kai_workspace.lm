% =====================================================================
% Example: a small multi-target C++ workspace described purely as
% facts. This file asserts data only — all resolution logic lives in
% prolog/targets.pl. The driver loads both files directly via
% PrologEngine::loadFile (see src/prolog_engine.cpp); there is no
% :- consult directive here because CppProlog's Interpreter::loadFile
% parses a file as a flat sequence of facts/rules, not as a script of
% executable directives — a bare ":- consult(...)." line is not valid
% content for it to load.
% =====================================================================

% --- configuration facts (set by the driver before resolution) ---
platform(linux).
% debug.               % uncomment for a debug configuration

% --- kai_language: header-only interface target ---
target(kai_language, interface).
include(kai_language, "examples/kai_workspace/CppKaiCore/include/").

% --- kai_core: the core library ---
target(kai_core, lib).
sources(kai_core, "examples/kai_workspace/CppKaiCore/src/*.cpp").
include(kai_core, "examples/kai_workspace/CppKaiCore/include/").
depends(kai_core, kai_language).
depends(kai_core, enet).
depends(kai_core, fmt, private).
define(kai_core, 'KAI_DEBUG') :- debug.

% --- enet: external dependency, itself has a platform-conditional link ---
target(enet, lib).
link(enet, ws2_32) :- platform(windows).
link(enet, winmm)  :- platform(windows).
link(enet, pthread) :- platform(linux).

% --- fmt: header-mostly, no extra links ---
target(fmt, lib).

% --- kai_node: the executable ---
target(kai_node, exe).
sources(kai_node, "examples/kai_workspace/node/*.cpp").
depends(kai_node, kai_core).

% Example queries once loaded (see README for driver invocation):
%   ?- depends_all(kai_node, D).
%   ?- resolved_link(kai_node, L).
%   ?- cyclic(kai_core).
%   ?- depends_on(fmt, T).
