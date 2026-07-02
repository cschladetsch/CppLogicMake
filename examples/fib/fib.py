#!/usr/bin/env python3
"""Python build of the shared fib demo: prints the sum of the first ten
Fibonacci numbers (== 88), matching the C++23, C++17 and Rust siblings so
the Fib.CrossLanguageEndToEnd test can assert cross-language agreement."""


def main() -> None:
    a, b, total = 0, 1, 0
    for _ in range(10):
        total += a
        a, b = b, a + b
    print(total)


if __name__ == "__main__":
    main()
