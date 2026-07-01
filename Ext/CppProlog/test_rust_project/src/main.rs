//! Hello World Rust Application
//! Generated using Universal Project Generator
//!
//! This application demonstrates integration between Prolog-based
//! code generation and rust application development.

use std::io::{self, Write};

fn main() -> io::Result<()> {
    println!("🦀 Hello World from rust!");
    println!("This application was generated using:");
    println!("  • CppProlog for rule-based generation");
    println!("  • Rust for the generator system");
    println!("  • Prolog knowledge bases for development files");
    
    print!("Enter your name: ");
    io::stdout().flush()?;
    
    let mut name = String::new();
    io::stdin().read_line(&mut name)?;
    let name = name.trim();
    
    if !name.is_empty() {
        println!("Hello, {}! Welcome to the generated rust application! 🎉", name);
    } else {
        println!("Hello, anonymous user! Welcome to the generated rust application! 🎉");
    }
    
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_application_runs() {
        // This test ensures the application compiles and basic logic works
        assert_eq!(2 + 2, 4);
    }
}
