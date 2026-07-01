% Prolog Docker Generator
% Generates optimized Dockerfiles using declarative rules
% Demonstrates advanced Prolog programming for infrastructure as code

% ============================================================================
% Docker Instruction Representation
% ============================================================================

% Docker instruction facts
docker_instruction(from(BaseImage, Alias)) :- atom(BaseImage), atom(Alias).
docker_instruction(from(BaseImage)) :- atom(BaseImage).
docker_instruction(run(Command)) :- atom(Command).
docker_instruction(copy(Source, Dest)) :- atom(Source), atom(Dest).
docker_instruction(workdir(Dir)) :- atom(Dir).
docker_instruction(env(Key, Value)) :- atom(Key), atom(Value).
docker_instruction(expose(Port)) :- number(Port).
docker_instruction(cmd(Command)) :- atom(Command).
docker_instruction(entrypoint(Command)) :- atom(Command).
docker_instruction(user(UserSpec)) :- atom(UserSpec).
docker_instruction(volume(Path)) :- atom(Path).
docker_instruction(label(Key, Value)) :- atom(Key), atom(Value).

% ============================================================================
% Build Stage Definitions
% ============================================================================

% Define build stages with their purposes
build_stage(builder, build_environment).
build_stage(runtime, production_runtime).
build_stage(development, dev_environment).
build_stage(benchmark, performance_testing).
build_stage(test, ci_testing).

% Stage dependencies - which stages depend on others
stage_depends_on(runtime, builder).
stage_depends_on(development, builder).
stage_depends_on(benchmark, runtime).
stage_depends_on(test, builder).

% ============================================================================
% Base Image Selection Rules
% ============================================================================

% Select optimal base image based on requirements
optimal_base_image(cpp_build, 'ubuntu:22.04') :-
    write('Selected Ubuntu 22.04 for C++ build compatibility'), nl.

optimal_base_image(minimal_runtime, 'ubuntu:22.04') :-
    write('Selected Ubuntu 22.04 for runtime - good library support'), nl.

optimal_base_image(alpine_runtime, 'alpine:3.18') :-
    write('Selected Alpine for minimal runtime - smaller image'), nl.

% Language-specific base images
language_base_image(cpp23, ubuntu, 'ubuntu:22.04').
language_base_image(cpp20, ubuntu, 'ubuntu:20.04').
language_base_image(cpp17, centos, 'centos:8').

% ============================================================================
% Package Installation Optimization
% ============================================================================

% Essential build packages for C++23
essential_build_packages([
    'build-essential',
    'cmake',
    'git',
    'ninja-build',
    'clang-15',
    'libc++-15-dev',
    'libc++abi-15-dev',
    'pkg-config'
]).

% Runtime packages (minimal set)
essential_runtime_packages([
    'libc++1-15',
    'libc++abi1-15'
]).

% Development packages (additional)
development_packages([
    'gdb',
    'valgrind',
    'strace',
    'htop',
    'vim',
    'curl',
    'wget'
]).

% Generate optimized package installation command
generate_package_install(PackageList, 'apt-get update && apt-get install -y && apt-get clean').

% ============================================================================
% Multi-Stage Build Optimization Rules
% ============================================================================

% Optimize layer caching by grouping related operations
cache_optimized_layers(Stage, Instructions) :-
    Stage = builder,
    Instructions = [
        '# Install build dependencies first (cached layer)',
        run('apt-get update && apt-get install -y build-essential cmake git'),
        '# Copy dependency files (better caching)',
        copy('CMakeLists.txt', '.'),
        copy('External/', 'External/'),
        '# Copy source files (changes more frequently)', 
        copy('src/', 'src/'),
        copy('tests/', 'tests/')
    ].

% Security optimization rules
security_optimized(Instructions) :-
    Instructions = [
        '# Create non-root user for security',
        run('groupadd -r prolog && useradd -r -g prolog prolog'),
        user('prolog')
    ].

% ============================================================================
% Build Strategy Selection
% ============================================================================

% Select build strategy based on requirements
build_strategy(fast_development, Strategy) :-
    Strategy = [
        from('ubuntu:22.04', builder),
        run('apt-get update && apt-get install -y build-essential cmake'),
        workdir('/workspace'),
        copy('.', '.'),
        run('cmake -B build && make -C build')
    ].

build_strategy(optimized_production, Strategy) :-
    Strategy = [
        from('ubuntu:22.04', builder),
        run('apt-get update && apt-get install -y build-essential cmake ninja-build'),
        workdir('/build'),
        copy('CMakeLists.txt', '.'),
        copy('External/', 'External/'),
        run('cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release'),
        copy('src/', 'src/'),
        run('ninja -C build'),
        from('ubuntu:22.04', runtime),
        copy('--from=builder /build/build/bin/', './bin/'),
        user('prolog'),
        cmd('./bin/prolog_interpreter')
    ].

build_strategy(ci_testing, Strategy) :-
    Strategy = [
        from('ubuntu:22.04'),
        run('apt-get update && apt-get install -y build-essential cmake'),
        workdir('/app'),
        copy('.', '.'),
        run('cmake -B build && make -C build'),
        run('cd build && ./bin/prolog_tests')
    ].

% ============================================================================
% Dockerfile Generation Engine
% ============================================================================

% Main predicate to generate complete Dockerfile
generate_dockerfile(ProjectType, Strategy) :-
    write('# Generated Dockerfile for '), write(ProjectType), nl,
    write('# Strategy: '), write(Strategy), nl,
    write('# Auto-generated by Prolog Docker Generator'), nl, nl,
    build_strategy(Strategy, Instructions),
    generate_instructions(Instructions).

% Generate individual instructions
generate_instructions([]).
generate_instructions([Instruction|Rest]) :-
    generate_instruction(Instruction),
    generate_instructions(Rest).

% Generate specific instruction types
generate_instruction(from(Image, Alias)) :-
    write('FROM '), write(Image), write(' AS '), write(Alias), nl.

generate_instruction(from(Image)) :-
    write('FROM '), write(Image), nl.

generate_instruction(run(Command)) :-
    write('RUN '), write(Command), nl.

generate_instruction(copy(Source, Dest)) :-
    write('COPY '), write(Source), write(' '), write(Dest), nl.

generate_instruction(workdir(Dir)) :-
    write('WORKDIR '), write(Dir), nl.

generate_instruction(env(Key, Value)) :-
    write('ENV '), write(Key), write('='), write(Value), nl.

generate_instruction(user(UserSpec)) :-
    write('USER '), write(UserSpec), nl.

generate_instruction(cmd(Command)) :-
    write('CMD '), write(Command), nl.

generate_instruction(expose(Port)) :-
    write('EXPOSE '), write(Port), nl.

generate_instruction(Comment) :-
    atom_chars(Comment, [#|_]),
    write(Comment), nl.

% ============================================================================
% Advanced Optimization Rules
% ============================================================================

% Minimize image layers by combining related RUN commands
optimize_layers(Instructions, OptimizedInstructions) :-
    combine_run_commands(Instructions, OptimizedInstructions).

combine_run_commands([], []).
combine_run_commands([run(Cmd1), run(Cmd2)|Rest], [run(CombinedCmd)|OptimizedRest]) :-
    atomic_list_concat([Cmd1, ' && ', Cmd2], '', CombinedCmd),
    combine_run_commands(Rest, OptimizedRest).
combine_run_commands([Other|Rest], [Other|OptimizedRest]) :-
    \+ Other = run(_),
    combine_run_commands(Rest, OptimizedRest).

% Security scanning rules
security_check(Instructions, Issues) :-
    findall(Issue, security_issue(Instructions, Issue), Issues).

security_issue(Instructions, root_user) :-
    \+ member(user(_), Instructions).

security_issue(Instructions, exposed_secrets) :-
    member(env(Key, Value), Instructions),
    ( sub_atom(Key, _, _, _, 'PASSWORD') ; 
      sub_atom(Key, _, _, _, 'SECRET') ;
      sub_atom(Key, _, _, _, 'TOKEN') ),
    write('Warning: Potential secret in environment variable: '), write(Key), nl.

% ============================================================================
% Performance Optimization Predicates
% ============================================================================

% Analyze and optimize for build performance
optimize_build_performance(Strategy, OptimizedStrategy) :-
    Strategy = optimized_production,
    OptimizedStrategy = [
        '# Use build cache mount for faster rebuilds',
        from('ubuntu:22.04', builder),
        run('apt-get update && apt-get install -y build-essential cmake ninja-build'),
        workdir('/build'),
        '# Copy dependencies first for better layer caching',
        copy('CMakeLists.txt', '.'),
        copy('External/', 'External/'),
        '# Configure build with cache mount',
        run('--mount=type=cache,target=/build/build cmake -B build -G Ninja'),
        copy('src/', 'src/'),
        run('--mount=type=cache,target=/build/build ninja -C build'),
        '# Multi-stage production image',
        from('ubuntu:22.04', runtime),
        run('apt-get update && apt-get install -y libc++1-15 && rm -rf /var/lib/apt/lists/*'),
        copy('--from=builder /build/build/bin/', './bin/'),
        user('prolog'),
        cmd('./bin/prolog_interpreter')
    ].

% ============================================================================
% Dockerfile Templates for Different Use Cases
% ============================================================================

% Template for CppLProlog specifically
cppprolog_dockerfile(production) :-
    write('# CppLProlog Production Dockerfile'), nl,
    write('# Multi-stage build for optimal size and performance'), nl, nl,
    generate_dockerfile(cppprolog, optimized_production).

cppprolog_dockerfile(development) :-
    write('# CppLProlog Development Dockerfile'), nl,
    write('# Includes debugging tools and development environment'), nl, nl,
    generate_dockerfile(cppprolog, fast_development).

% Template for CI/CD pipeline
ci_dockerfile :-
    write('# CppLProlog CI/CD Dockerfile'), nl,
    write('# Optimized for automated testing'), nl, nl,
    generate_dockerfile(cppprolog, ci_testing).

% ============================================================================
% Interactive Dockerfile Builder
% ============================================================================

% Interactive builder that asks questions and generates Dockerfile
build_interactive_dockerfile :-
    write('CppLProlog Docker Generator'), nl,
    write('============================'), nl,
    write('What type of Dockerfile do you want to generate?'), nl,
    write('1. Production (optimized, multi-stage)'), nl,
    write('2. Development (includes dev tools)'), nl,
    write('3. CI/CD (testing focused)'), nl,
    write('Choice (1-3): '),
    read(Choice),
    generate_choice_dockerfile(Choice).

generate_choice_dockerfile(1) :-
    nl, write('Generating production Dockerfile...'), nl, nl,
    cppprolog_dockerfile(production).

generate_choice_dockerfile(2) :-
    nl, write('Generating development Dockerfile...'), nl, nl,
    cppprolog_dockerfile(development).

generate_choice_dockerfile(3) :-
    nl, write('Generating CI/CD Dockerfile...'), nl, nl,
    ci_dockerfile.

% ============================================================================
% Example Queries
% ============================================================================

% To generate a production Dockerfile:
% ?- cppprolog_dockerfile(production).

% To run interactive builder:
% ?- build_interactive_dockerfile.

% To check security issues:
% ?- build_strategy(optimized_production, S), security_check(S, Issues).

% To optimize build performance:
% ?- optimize_build_performance(optimized_production, Optimized).

% ============================================================================
% Utility Predicates
% ============================================================================

% Helper to estimate image size based on packages
estimate_image_size(PackageList, SizeMB) :-
    length(PackageList, PackageCount),
    SizeMB is PackageCount * 15 + 200.  % Rough estimation

% Helper to validate Dockerfile syntax
validate_dockerfile_syntax(Instructions) :-
    forall(member(Instruction, Instructions), 
           docker_instruction(Instruction)).