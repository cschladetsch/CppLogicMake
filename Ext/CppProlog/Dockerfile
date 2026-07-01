# Multi-stage Dockerfile for CppLProlog
# Optimized for build caching and minimal production image

# ============================================================================
# Stage 1: Build Environment
# ============================================================================
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    clang-15 \
    libc++-15-dev \
    libc++abi-15-dev \
    pkg-config \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set environment variables for modern C++
ENV CC=clang-15
ENV CXX=clang++-15
ENV CMAKE_BUILD_TYPE=Release

# Create build directory
WORKDIR /build

# Copy dependency files first (better layer caching)
COPY CMakeLists.txt .
COPY External/ External/

# Initialize submodules and fetch dependencies
RUN git init && \
    git submodule add https://github.com/agauniyal/rang.git External/rang || true

# Copy source files
COPY src/ src/
COPY tests/ tests/
COPY examples/ examples/
COPY benchmarks/ benchmarks/

# Configure and build
RUN cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=clang++-15 \
    -DCMAKE_C_COMPILER=clang-15 \
    -DCMAKE_CXX_STANDARD=23 \
    && ninja -C build

# Run tests to ensure build quality
RUN cd build && ./bin/prolog_tests

# ============================================================================
# Stage 2: Runtime Environment (Minimal)
# ============================================================================
FROM ubuntu:22.04 AS runtime

# Install minimal runtime dependencies
RUN apt-get update && apt-get install -y \
    libc++1-15 \
    libc++abi1-15 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user for security
RUN groupadd -r prolog && useradd -r -g prolog prolog

# Create application directory
WORKDIR /app

# Copy built executables from builder stage
COPY --from=builder /build/build/bin/ ./bin/
COPY --from=builder /build/examples/*.pl ./examples/

# Copy demo script
COPY demo.sh .
RUN chmod +x demo.sh

# Set ownership
RUN chown -R prolog:prolog /app

# Switch to non-root user
USER prolog

# Default command (interactive interpreter)
CMD ["./bin/prolog_interpreter"]

# ============================================================================
# Stage 3: Development Environment
# ============================================================================
FROM builder AS development

# Install additional development tools
RUN apt-get update && apt-get install -y \
    gdb \
    valgrind \
    strace \
    htop \
    vim \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Keep source code for development
WORKDIR /workspace
COPY . .

# Build in debug mode for development
RUN cmake -B build-debug -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=clang++-15 \
    -DCMAKE_C_COMPILER=clang-15 \
    -DCMAKE_CXX_STANDARD=23 \
    && ninja -C build-debug

# Default command for development
CMD ["/bin/bash"]

# ============================================================================
# Stage 4: Benchmarking Environment
# ============================================================================
FROM runtime AS benchmark

# Copy benchmark executables
COPY --from=builder /build/build/bin/prolog_benchmarks ./bin/

# Create benchmark results directory
RUN mkdir -p /app/benchmark-results

# Default command runs benchmarks
CMD ["./bin/prolog_benchmarks", "--benchmark_format=json", "--benchmark_out=/app/benchmark-results/results.json"]