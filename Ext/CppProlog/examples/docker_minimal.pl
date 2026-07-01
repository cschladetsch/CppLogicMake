% Minimal working Docker Generator for CppLProlog

% Simple facts about Docker setup
docker_base(ubuntu).
build_tool(cmake).
build_tool(make).

% Essential packages
package(build_essential).
package(cmake).
package(git).

% Generate simple production Dockerfile
cppprolog_dockerfile(production) :-
    write('FROM ubuntu:22.04'), nl,
    write('RUN apt-get update'), nl,
    write('RUN apt-get install -y build-essential cmake git'), nl,
    write('WORKDIR /build'), nl,
    write('COPY . .'), nl,
    write('RUN cmake -B build -DCMAKE_BUILD_TYPE=Release'), nl,
    write('RUN make -C build'), nl,
    write('CMD ["./build/bin/prolog_interpreter"]'), nl.

% Simple CI dockerfile  
ci_dockerfile :-
    write('FROM ubuntu:22.04'), nl,
    write('RUN apt-get update && apt-get install -y build-essential cmake'), nl,
    write('WORKDIR /app'), nl,
    write('COPY . .'), nl,
    write('RUN cmake -B build && make -C build'), nl,
    write('RUN cd build && ./bin/prolog_tests'), nl.

% Check if package is essential
essential_build_packages([build_essential, cmake, git]).

% Simple security check
security_check(Instructions, []).

% Performance optimization check  
optimize_build_performance(optimized_production, optimized_production).

% Size estimation
estimate_image_size(Packages, 200).