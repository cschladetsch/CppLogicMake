// Rust build of the shared fib demo: prints the sum of the first ten
// Fibonacci numbers (== 88), matching the C++23, C++17 and Python
// siblings. The Fib.CrossLanguageEndToEnd test compiles this with
// `rustc fib.rs` when a Rust toolchain is present, and skips it
// (reporting SKIP) when rustc is not on PATH.
fn main() {
    let (mut a, mut b, mut total): (i64, i64, i64) = (0, 1, 0);
    for _ in 0..10 {
        total += a;
        let next = a + b;
        a = b;
        b = next;
    }
    println!("{}", total);
}
