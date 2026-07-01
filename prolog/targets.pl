% =====================================================================
% CppLogicMate — core build-graph knowledge base
%
% A build description is nothing more than a set of facts about
% targets, their sources, and their dependencies, plus a handful of
% rules for what follows from those facts. This file defines the
% *schema* (predicate shapes) and the *derived* rules. Your actual
% project description lives in a separate .pl file (see examples/)
% that only ever asserts facts — it never needs to touch this file.
% =====================================================================

% --- schema (documentation only, Prolog doesn't enforce arity here) ---
% target(Name, Kind).            Kind = lib | exe | interface
% sources(Name, Glob).
% include(Name, Path).
% depends(Name, Dep).            direct dependency, public by default
% depends(Name, Dep, private).   private dependency
% define(Name, Macro).
% define(Name, Macro) :- Guard.  conditional define
% link(Name, Lib) :- Guard.      conditional link (platform libs etc.)
% platform(windows). / platform(linux). / platform(macos).
% debug.                         asserted only in debug configs
% cross_compiling.

% --- transitive dependency closure ---
% depends_all(Target, Dep) succeeds for every dependency Target pulls
% in, directly or transitively, public or private. This is the whole
% reason to prefer a real resolution engine over a hand-rolled DFS:
% the rule is two lines and needs no maintenance as the graph grows.

depends_all(T, D) :- depends(T, D).
depends_all(T, D) :- depends(T, D, private).
depends_all(T, D) :- depends(T, X), depends_all(X, D).
depends_all(T, D) :- depends(T, X, private), depends_all(X, D).

% --- public-only closure (for computing transitive PUBLIC link deps,
%     which is the CMake-relevant distinction: private deps of a
%     dependency should not leak into a target's own public interface) ---

depends_public(T, D) :- depends(T, D).
depends_public(T, D) :- depends(T, X), depends_public(X, D).

% --- cycle detection ---
% A cyclic depends_all query will not terminate in a naive engine, so
% cycle checking is done explicitly via bounded reachability instead
% of relying on non-termination as a signal.

reaches(T, D) :- depends(T, D).
reaches(T, D) :- depends(T, D, private).
reaches(T, D) :- depends(T, X), reaches(X, D).
reaches(T, D) :- depends(T, X, private), reaches(X, D).

cyclic(T) :- reaches(T, T).

% --- link resolution ---
% Everything a target ultimately needs to link against: its own
% explicit links, plus every transitive dependency's own links that
% survive their guards (platform, debug, etc.).

resolved_link(T, L) :- link(T, L).
resolved_link(T, L) :- depends_all(T, D), link(D, L).

% --- macro/define resolution, same shape as link resolution ---

resolved_define(T, M) :- define(T, M).
resolved_define(T, M) :- depends_all(T, D), define(D, M).

% --- what breaks if a dependency is removed ---
% Answers: "which targets transitively depend on X?"

depends_on(D, T) :- depends_all(T, D).
