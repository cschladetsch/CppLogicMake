// C++17 build of the shared fib demo: prints the sum of the first ten
// Fibonacci numbers (== 88). The static_assert pins the standard to
// exactly C++17 — if logimake failed to emit a per-target CXX_STANDARD
// and the file-level default (C++23) applied instead, this would fail to
// compile, which is precisely the regression the test guards against.
#include <cstdint>
#include <iostream>

static_assert(__cplusplus == 201703L, "fib_cpp17 must be built as C++17");

int main() {
    std::int64_t a = 0, b = 1, sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += a;
        const std::int64_t next = a + b;
        a = b;
        b = next;
    }
    std::cout << sum << '\n';
    return 0;
}
