# Nova Compiler Architecture

This document describes the internal architecture of the `novac` compiler.

## Pipeline Overview

The compiler follows a traditional frontend-middleend-backend structure.
1. **Lexical Analysis (Scanner)**
2. **Syntax Analysis (Parser)**
3. **Semantic Analysis (Type Checker)**
4. **Code Generation (LLVM IR)**
5. **Native Linking**

### 1. Lexical Analysis (`src/lexer.c`)
The lexer reads the raw ASCII source code and groups characters into meaningful `Token`s. It strips out whitespace and comments. Errors at this stage are usually "Unexpected character".

### 2. Syntax Analysis (`src/parser.c`)
The parser is a Hand-Written Recursive Descent Parser. It consumes the stream of tokens and builds an Abstract Syntax Tree (AST).
- The AST structures are defined in `include/ast.h`.
- The parser implements "Panic Mode" error recovery. If it encounters a syntax error, it synchronizes to the next statement boundary (like a `;` or a `}`), preventing an avalanche of cascading syntax errors.

### 3. Semantic Analysis (`src/sema.c`)
This phase traverses the AST to check for semantic rules that cannot be captured by the grammar.
- **Symbol Table:** Uses a scoped linked-list environment to track variable and function declarations.
- **Type Checking:** Ensures operations are performed on compatible types (e.g., checking that a `void` function doesn't return an `int`, or that an undeclared variable is not used).

### 4. Code Generation (`src/codegen.c`)
Once the AST is proven valid, the compiler traverses it one last time to emit LLVM Intermediate Representation (`.ll`).
- It translates Nova constructs into LLVM IR instructions (e.g., `add`, `fmul`, `icmp`).
- It handles memory allocation for local variables using `alloca` and `store`/`load` instructions, delegating register allocation and optimization to the LLVM backend.

### 5. Driver (`src/main.c`)
The driver orchestrates the whole process.
- After `codegen` produces `output.ll`, the driver invokes LLVM's `llc` tool via the system shell to compile the IR into a machine-specific object file (`output.o`).
- Finally, it invokes `clang` (or `gcc`) to link the object file with the C standard library (giving us access to the OS entry point) and produce the final native executable (`.exe`).
