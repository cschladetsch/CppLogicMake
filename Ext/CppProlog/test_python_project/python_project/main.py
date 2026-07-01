"""Main module for Python Project
Generated using Universal Project Generator
"""

import sys


def main():
    """Main entry point for the application."""
    print("🐍 Hello World from Python!")
    print("This application was generated using:")
    print("  • CppProlog for rule-based generation")
    print("  • Rust for the generator system")
    print("  • Prolog knowledge bases for development files")
    
    name = input("Enter your name: ").strip()
    
    if name:
        print(f"Hello, {name}! Welcome to the generated Python application! 🎉")
    else:
        print("Hello, anonymous user! Welcome to the generated Python application! 🎉")


if __name__ == "__main__":
    main()
