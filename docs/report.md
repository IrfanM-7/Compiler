# Project Report: Nova DSL Compiler

## 1. Introduction
Nova is a statically-typed, C-like Domain-Specific Language designed for high-performance systems programming. This report documents the design and implementation of a modular, end-to-end compiler for Nova, built entirely in C and targeting LLVM.

## 2. Compilation Pipeline

### 2.1 Lexical Analysis (Phase 1)
The lexer is implemented as a manual state-machine scanner. It categorizes the source character stream into meaningful tokens (identifiers, keywords, literals, operators).
- **Tool**: Manual C Implementation.
- **Key Features**: Supports nested comments, escape sequences in strings, and complex numeric literals.

### 2.2 Parsing (Phase 2)
The parser uses a **Recursive Descent** strategy to convert the token stream into an Abstract Syntax Tree (AST).
- **Grammar**: Statically defined in the code logic.
- **AST Construction**: Nodes are dynamically allocated and structured hierarchically to represent the program's logic.

### 2.3 Semantic Analysis (Phase 3)
This phase ensures the program follows the language's rules.
- **Symbol Table**: Keeps track of variable and function declarations across scopes.
- **Type Checking**: Validates that operations are performed on compatible types (e.g., preventing string multiplication).
- **Scope Resolution**: Ensures variables are defined before use and handles nested block scoping.

### 2.4 Code Generation (Phase 4 & 5)
The compiler features a dual-backend strategy:
1. **LLVM Backend**: Generates optimized LLVM Static IR (`.ll`). It targets the `llc` backend for native code generation.
2. **C Fallback**: For environments without LLVM, the compiler transpiles AST nodes directly into C code and compiles them using a local C compiler (MSVC/Clang/GCC).

## 3. Advanced Features
- **Array Support**: Dynamic allocation and indexing for integers and strings.
- **Standard Library**: Built-in I/O functions like `print_string` and `input_int`.
- **Portability**: Automated fallback mechanism ensures the compiler works on any platform with a standard C compiler.

## 4. Conclusion
The Nova Compiler successfully demonstrates a complete compilation lifecycle. By avoiding third-party generator tools (Flex/Bison), the project showcases a deep understanding of compiler internals and modular software architecture.

---
**Author**: Syed Irfan M  
**Register No**: RA2311026050121  
**Date**: April 2026
