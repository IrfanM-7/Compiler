# PROJECT REPORT: NOVA DSL COMPILER

**Submitted by:** Syed Irfan M & Arun Rishvanth  
**Register No:** RA2311026050121 & RA2311026050122  
**Course:** Compiler Design  
**Date:** April 2026  

---

## 1. Abstract
The **Nova DSL Compiler** is a high-performance, modular compiler pipeline designed to translate a custom statically-typed Domain-Specific Language (DSL) into LLVM Intermediate Representation (IR). Built entirely from scratch in C99, the compiler implements a complete frontend-to-backend workflow, including a manual recursive-descent parser, a scoped symbol table for semantic analysis, and an LLVM-compliant code generation engine. The project demonstrates the application of modern compiler construction techniques such as panic-mode error recovery and multi-pass semantic analysis to ensure robustness and efficiency.

## 2. Introduction
In modern software engineering, Domain-Specific Languages (DSLs) provide tailored abstractions that improve developer productivity and system safety. The **Nova** language is designed for systems programming with a focus on simplicity and safety, featuring a C-like syntax with enhanced type safety for arrays and strings.

### 2.1 Objectives
- To design a statically-typed language with support for fundamental data types (int, float, string) and arrays.
- To implement a robust compiler frontend using C99 without external generator tools (Lex/Yacc).
- To leverage LLVM as a backend for native code optimization and cross-platform execution.
- To provide meaningful error reporting and recovery mechanisms.

## 3. System Architecture
The compiler follows a traditional 5-phase pipeline:

1.  **Lexical Analysis (Scanner):** Converts raw ASCII text into a stream of categorized tokens.
2.  **Syntax Analysis (Parser):** Consumes tokens to produce an Abstract Syntax Tree (AST) using recursive descent.
3.  **Semantic Analysis (Sema):** Verifies the AST against language rules (type checking, scope resolution).
4.  **Intermediate Representation (Codegen):** Translates the verified AST into LLVM IR instructions.
5.  **Native Compilation:** Uses the LLVM backend (`llc` and `clang`) to produce the final native binary.

## 4. Detailed Design

### 4.1 Lexical Analysis (`lexer.c`)
The lexer is a manual scanner that uses a single-pass approach to identify keywords (`fn`, `let`, `if`, `while`), operators (`+`, `-`, `*`, `/`, `->`), and literals. It handles comments and whitespace while tracking line numbers for accurate error reporting.

### 4.2 Syntax Analysis (`parser.c`)
The parser is implemented as a **Hand-Written Recursive Descent Parser**. 
- **AST Generation:** It builds a hierarchical tree representing the program structure, where nodes are defined for expressions, statements, and declarations.
- **Error Recovery (Panic Mode):** When a syntax error is encountered, the parser enters a "panic mode" where it suppresses further errors and synchronizes to the next logical statement boundary (e.g., a semicolon or a keyword like `fn`). This prevents a single error from causing a cascade of false reports.

### 4.3 Semantic Analysis (`sema.c`)
This phase ensures that the program is logically sound.
- **Symbol Table:** Implemented as a linked list of `Scope` objects, each containing a list of `Symbol` definitions. This allows for nested scopes (global, function, and block-level).
- **Two-Pass Analysis:** 
    - **Pass 1:** Scans all function declarations to populate the global symbol table, enabling mutual recursion.
    - **Pass 2:** Analyzes the bodies of functions, performing type inference and validation for every expression.

### 4.4 Code Generation (`codegen.c`)
The backend translates Nova AST nodes into LLVM IR.
- **Memory Management:** Local variables are handled using the `alloca` instruction, ensuring stack-based allocation.
- **Intrinsic Functions:** Standard library functions like `print_int` and `strcmp` are mapped to C-runtime equivalents during the linking phase.

## 5. Implementation Details
- **Language:** C99
- **Backend:** LLVM 15+
- **Build System:** CMake
- **Tools Used:** MSVC/GCC, LLVM LLC, Clang.

## 6. Results and Testing
The compiler has been validated against several complex algorithms.

### 6.1 Sample: Factorial (Recursion)
The following Nova code calculates the factorial of a number using recursion:

```c
fn factorial(n: int) -> int {
    if (n < 2) {
        return 1;
    }
    return n * factorial(n - 1);
}

fn main() -> int {
    let result: int = factorial(5);
    return result;
}
```
**Output:** The program correctly returns `120`.

### 6.2 Sample: Bubble Sort (Complex Logic)
Nova supports string manipulation and array indexing, as seen in this bubble sort implementation:

```c
fn main() -> int {
    let arr: string[] = allocate_string_array(3);
    arr[0] = "zebra";
    arr[1] = "apple";
    arr[2] = "monkey";
    
    // Nested while-loop sorting logic...
    // (Logic omitted for brevity)
    
    print_string(arr[0]); // Output: "apple"
    return 0;
}
```

## 7. Conclusion and Future Scope
The Nova DSL Compiler successfully demonstrates the power of a custom-built pipeline targeting LLVM. It provides a solid foundation for further extensions such as:
- **Structs and User-Defined Types:** To allow for complex data modeling.
- **Optimization Passes:** Implementing custom AST-level optimizations before IR generation.
- **Garbage Collection:** Adding a runtime system for automatic memory management of strings and arrays.

---
**Date:** 26th April 2026  
**Place:** University Campus  
**Signature:** Syed Irfan M & Arun Rishvanth  
