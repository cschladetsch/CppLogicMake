// C++23 build of the shared fib demo: prints the sum of the first ten
// Fibonacci numbers (== 88). The static_assert gates compilation on the
// standard actually being C++23, so this file is also the end-to-end
// proof that logimake emitted a per-target CXX_STANDARD of 23.
#include <cstdint>
#include <iostream>

static_assert(__cplusplus >= 202302L, "fib_cpp23 must be built as C++23");

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
