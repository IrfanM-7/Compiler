# 🌌 Nova DSL Compiler

> **A High-Performance, Modular Compiler Pipeline Targeting LLVM IR**

[![Language: Pure C](https://img.shields.io/badge/Language-Pure%20C-007ACC.svg?style=for-the-badge&logo=c)](https://en.wikipedia.org/wiki/C99)
[![Backend: LLVM IR](https://img.shields.io/badge/Backend-LLVM%20IR-A6DA29.svg?style=for-the-badge&logo=llvm)](https://llvm.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-F7DF1E.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)
[![Build: CMake](https://img.shields.io/badge/Build-CMake-064F8C.svg?style=for-the-badge&logo=cmake)](https://cmake.org/)

Nova is a statically-typed Domain-Specific Language (DSL) meticulously engineered for high-performance systems programming. This repository features a full-stack compiler architecture built entirely from scratch in **C99**, bridging the gap between high-level logic and native machine code via the LLVM optimization engine.

---

## 🚀 Key Features

*   **⚡ Zero Dependencies**: Built without Lex/Yacc or other generator tools.
*   **🛠️ Hand-Written Frontend**: Manual recursive-descent parser with **Panic-Mode Error Recovery**.
*   **🔍 Semantic Intelligence**: Multi-pass analysis with a hierarchical scoped symbol table.
*   **💎 LLVM IR Integration**: Generates highly optimized `.ll` files for cross-platform native execution.
*   **📊 Industrial Pipeline**: Implements Lexing, Parsing, AST Generation, Semantic Validation, and IR Generation.

---

## 🏗️ Architecture Pipeline

| Stage | Component | Output |
| :--- | :--- | :--- |
| **Lexical Analysis** | `lexer.c` | Categorized Token Stream |
| **Syntax Analysis** | `parser.c` | Abstract Syntax Tree (AST) |
| **Semantic Analysis** | `sema.c` | Verified Program State & Symbol Table |
| **Code Generation** | `codegen.c` | Optimized LLVM IR (`.ll`) |
| **Native Compilation** | `llc` / `clang` | Native Executable (`.exe` / `.out`) |

---

## 📂 Project Structure

```bash
Nova-Compiler/
├── 📁 src/         # Core Implementation (Lexer, Parser, Sema, Codegen)
├── 📁 include/     # Header files & AST definitions
├── 📁 tests/       # Test suites (Recursion, Sorting, Fibonacci)
├── 📁 docs/        # Technical Specifications & Project Reports
└── 📄 CMakeLists.txt # Build configuration
```

---

## 🛠️ Installation & Usage

### 1. Build the Compiler
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 2. Run your Nova Script
Create `main.nv`:
```c
fn factorial(n: int) -> int {
    if (n < 2) { return 1; }
    return n * factorial(n - 1);
}

fn main() -> int {
    let result: int = factorial(5);
    print_int(result); // Output: 120
    return 0;
}
```

Compile and execute:
```bash
.\Release\novac.exe main.nv -o main.exe
.\main.exe
```

---

## 📜 Academic Context

This project was developed as part of the **Compiler Design** curriculum to demonstrate the implementation of a full-scale language processor.

### 👤 Author Details
- **Name**: Syed Irfan M & Arun Rishvanth
- **Register Number**: RA2311026050121 & RA2311026050122
- **University**: SRM Institute of Science and Technology
- **Year**: 2026

---

## ⚖️ License
Distributed under the MIT License. See `LICENSE` for more information.

---
<p align="center">
  Developed with ❤️ by <a href="https://github.com/IrfanM-7">Syed Irfan M</a> & <a href="https://github.com/arunrs2210">Arun Rishvanth</a>
</p>
