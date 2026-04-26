#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (!buffer) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

static void printUsage() {
    printf("Usage: novac <input.nv> [-o <output_executable>]\n");
}

int main(int argc, const char* argv[]) {
    printf("============================================================\n");
    printf("        Nova DSL Compiler - Version 1.0.0\n");
    printf("============================================================\n\n");

    if (argc < 2) {
        printUsage();
        return 64;
    }

    const char* inputPath = argv[1];
    const char* outputPath = "a.exe"; // default output for Windows
    const char* llPath = "output.ll";

    if (argc >= 4 && strcmp(argv[2], "-o") == 0) {
        outputPath = argv[3];
    }

    printf("----- Phase 1: Reading Source File -----\n");
    char* source = readFile(inputPath);
    printf("[DONE] Source file loaded.\n\n");

    printf("----- Phase 2: Lexical Analysis & Parsing -----\n");
    Program* program = parse(source);
    if (!program) {
        fprintf(stderr, "[ERROR] Compilation halted due to syntax errors.\n");
        free(source);
        return 65;
    }
    printf("[DONE] Abstract Syntax Tree (AST) constructed successfully.\n\n");

    printf("----- Phase 3: Semantic Analysis -----\n");
    if (!analyze(program)) {
        fprintf(stderr, "[ERROR] Compilation halted due to semantic errors.\n");
        free(source);
        return 65;
    }
    printf("[DONE] No semantic errors found. Symbol table verified.\n\n");

    printf("----- Phase 4: Code Generation (LLVM IR) -----\n");
    generateCode(program, llPath);
    // Also generate C code as a fallback
    generateCCode(program, "output.c");
    printf("[DONE] LLVM IR emitted to %s\n\n", llPath);

    printf("----- Phase 5: Native Binary Compilation -----\n");
    char command[512];
    
    snprintf(command, sizeof(command), "llc -filetype=obj %s -o output.o", llPath);
    if (system(command) != 0) {
        printf("[NOTICE] 'llc' not found. Triggering Fallback Pipeline (C + MSVC)...\n");
        
        system("mkdir fallback_src 2>nul");
        system("move output.c fallback_src\\output.c >nul 2>nul");
        
        FILE* cmakeFile = fopen("fallback_src\\CMakeLists.txt", "w");
        fprintf(cmakeFile, "cmake_minimum_required(VERSION 3.10)\n");
        fprintf(cmakeFile, "project(NovaOutput C)\n");
        fprintf(cmakeFile, "add_executable(NovaExec output.c)\n");
        fclose(cmakeFile);
        
        system("\"C:\\Program Files\\CMake\\bin\\cmake.exe\" -S fallback_src -B fallback_build >nul 2>nul");
        if (system("\"C:\\Program Files\\CMake\\bin\\cmake.exe\" --build fallback_build --config Release >nul 2>nul") == 0) {
            snprintf(command, sizeof(command), "copy fallback_build\\Release\\NovaExec.exe %s >nul 2>nul", outputPath);
            system(command);
            printf("[SUCCESS] Compiled to %s (via C backend)\n", outputPath);
        } else {
            fprintf(stderr, "[ERROR] Fallback compilation failed.\n");
            free(source);
            return 1;
        }
    } else {
        snprintf(command, sizeof(command), "clang output.o -o %s", outputPath);
        if (system(command) != 0) {
            snprintf(command, sizeof(command), "gcc output.o -o %s", outputPath);
            if (system(command) != 0) {
                fprintf(stderr, "[ERROR] Could not link object file.\n");
                free(source);
                return 1;
            }
        }
        printf("[SUCCESS] Compiled to %s (via LLVM backend)\n", outputPath);
    }

    printf("\n============================================================\n");
    printf("         Compilation Finished Successfully.\n");
    printf("============================================================\n");

    free(source);
    return 0;
}
