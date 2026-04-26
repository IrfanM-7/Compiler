# Nova DSL Compiler
> A High-Performance, Modular Compiler Pipeline Targeting LLVM IR

![Language: Pure C](https://img.shields.io/badge/Language-Pure%20C-blue.svg)
![Backend: LLVM IR](https://img.shields.io/badge/Backend-LLVM%20IR-green.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Build: CMake](https://img.shields.io/badge/Build-CMake-orange.svg)

Nova is a custom statically-typed Domain-Specific Language (DSL) designed for systems programming. This repository implements a complete, end-to-end compiler built from scratch in C99, featuring a manual recursive-descent parser and a sophisticated code generation engine.

---

## Project Overview

Nova is designed for developers who need a lightweight, C-like language with modern safety features. The compiler is modular and handles every phase of the compilation process manually, ensuring maximum control over performance and error reporting.

### Compilation Phases

| Phase | Responsibility | Output |
| :--- | :--- | :--- |
| **1. Lexical Analysis** | Manual Scanner | Token Stream |
| **2. Parsing** | Recursive Descent | Abstract Syntax Tree (AST) |
| **3. Semantic Analysis** | Symbol Table & Type Check | Verified Program State |
| **4. Intermediate Code** | LLVM IR Generation | .ll Static IR File |
| **5. Native Compilation** | LLVM Backend (llc) | Native .exe Binary |

---

## Repository Structure

| Folder | Description |
| :--- | :--- |
| src/ | Core Engine: Lexer, Parser, AST nodes, and Codegen. |
| include/ | Public header definitions for compiler modularity. |
| tests/ | Comprehensive test suite featuring recursion, sorting, and arithmetic. |
| docs/ | Detailed academic reports and architecture specs. |
| build/ | CMake build artifacts and generated binaries. |

---

## Getting Started

### 1. Installation
Ensure you have CMake and a C Compiler (MSVC/GCC) installed.
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 2. Compile Your First Program
Create a file `hello.nv`:
```c
fn main() -> int {
    print_string("Hello, Nova World!");
    return 0;
}
```

Compile it:
```bash
.\Release\novac.exe hello.nv -o hello.exe
```

---

## Sample: Bubble Sort with Strings
Nova is powerful enough to handle complex algorithms. Below is a snippet of a string-sorting program demonstrating the language's capabilities:

```c
fn main() -> int {
    let arr: string[] = allocate_string_array(3);
    arr[0] = "zebra";
    arr[1] = "apple";
    arr[2] = "monkey";
    
    // Automatic sorting call
    sort_strings(arr, 3);
    
    print_string(arr[0]); // Prints "apple"
    return 0;
}
```

---

## Author Details
**Name**: Syed Irfan M  
**Register No**: RA2311026050121  
**Course**: Compiler Design  
**Date**: April 2026

---

## License
This project is licensed under the MIT License - see the LICENSE file for details.
