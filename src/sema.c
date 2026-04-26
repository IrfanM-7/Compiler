#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sema.h"

// Simple linked-list symbol table for this example
typedef struct Symbol {
    const char* name;
    int length;
    DataType type;
    int isFunction;
    struct Symbol* next;
} Symbol;

typedef struct Scope {
    Symbol* symbols;
    struct Scope* enclosing;
    DataType currentFunctionReturnType;
} Scope;

static Scope* currentScope = NULL;
static int semaError = 0;

static void errorAt(Token name, const char* message) {
    fprintf(stderr, "[line %d] Semantic Error at '%.*s': %s\n", 
            name.line, name.length, name.start, message);
    semaError = 1;
}

static void pushScope(DataType returnType) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->symbols = NULL;
    scope->enclosing = currentScope;
    if (currentScope && returnType == TYPE_UNKNOWN) {
        scope->currentFunctionReturnType = currentScope->currentFunctionReturnType;
    } else {
        scope->currentFunctionReturnType = returnType;
    }
    currentScope = scope;
}

static void popScope() {
    Scope* scope = currentScope;
    currentScope = scope->enclosing;
    
    Symbol* sym = scope->symbols;
    while (sym) {
        Symbol* next = sym->next;
        free(sym);
        sym = next;
    }
    free(scope);
}

static void defineSymbol(Token name, DataType type, int isFunction) {
    // Check if already defined in current scope
    Symbol* sym = currentScope->symbols;
    while (sym) {
        if (sym->length == name.length && memcmp(sym->name, name.start, name.length) == 0) {
            errorAt(name, "Symbol already defined in this scope.");
            return;
        }
        sym = sym->next;
    }
    
    sym = (Symbol*)malloc(sizeof(Symbol));
    sym->name = name.start;
    sym->length = name.length;
    sym->type = type;
    sym->isFunction = isFunction;
    sym->next = currentScope->symbols;
    currentScope->symbols = sym;
}

static Symbol* resolveSymbol(Token name) {
    Scope* scope = currentScope;
    while (scope) {
        Symbol* sym = scope->symbols;
        while (sym) {
            if (sym->length == name.length && memcmp(sym->name, name.start, name.length) == 0) {
                return sym;
            }
            sym = sym->next;
        }
        scope = scope->enclosing;
    }
    return NULL;
}

static void analyzeExpr(Expr* expr);
static void analyzeStmt(Stmt* stmt);

static void analyzeExpr(Expr* expr) {
    if (!expr) return;
    
    switch (expr->type) {
        case EXPR_LITERAL_INT:
            expr->dataType = TYPE_INT;
            break;
        case EXPR_LITERAL_FLOAT:
            expr->dataType = TYPE_FLOAT;
            break;
        case EXPR_LITERAL_STRING:
            expr->dataType = TYPE_STRING;
            break;
        case EXPR_VARIABLE: {
            Symbol* sym = resolveSymbol(expr->as.variable.name);
            if (!sym) {
                errorAt(expr->as.variable.name, "Undeclared variable.");
                expr->dataType = TYPE_UNKNOWN;
            } else {
                expr->dataType = sym->type;
            }
            break;
        }
        case EXPR_BINARY:
            analyzeExpr(expr->as.binary.left);
            analyzeExpr(expr->as.binary.right);
            // Simplistic type checking: int + int -> int, etc.
            if (expr->as.binary.left->dataType == TYPE_FLOAT || expr->as.binary.right->dataType == TYPE_FLOAT) {
                expr->dataType = TYPE_FLOAT;
            } else {
                expr->dataType = TYPE_INT;
            }
            // If it's a comparison, type is always boolean-like (int 1 or 0)
            switch (expr->as.binary.op.type) {
                case TOKEN_EQUAL_EQUAL:
                case TOKEN_BANG_EQUAL:
                case TOKEN_LESS:
                case TOKEN_LESS_EQUAL:
                case TOKEN_GREATER:
                case TOKEN_GREATER_EQUAL:
                    expr->dataType = TYPE_INT;
                    break;
                default: break;
            }
            break;
        case EXPR_CALL: {
            Symbol* sym = resolveSymbol(expr->as.call.callee);
            if (!sym) {
                errorAt(expr->as.call.callee, "Undeclared function.");
                expr->dataType = TYPE_UNKNOWN;
            } else if (!sym->isFunction) {
                errorAt(expr->as.call.callee, "Not a function.");
                expr->dataType = TYPE_UNKNOWN;
            } else {
                expr->dataType = sym->type;
            }
            for (int i = 0; i < expr->as.call.argCount; i++) {
                analyzeExpr(expr->as.call.arguments[i]);
            }
            break;
        }
        case EXPR_ARRAY_INDEX: {
            analyzeExpr(expr->as.arrayIndex.array);
            analyzeExpr(expr->as.arrayIndex.index);
            if (expr->as.arrayIndex.index->dataType != TYPE_INT) {
                errorAt(expr->as.arrayIndex.array->as.variable.name, "Array index must be an integer.");
            }
            // Determine resulting type
            switch (expr->as.arrayIndex.array->dataType) {
                case TYPE_INT_ARRAY: expr->dataType = TYPE_INT; break;
                case TYPE_FLOAT_ARRAY: expr->dataType = TYPE_FLOAT; break;
                case TYPE_STRING_ARRAY: expr->dataType = TYPE_STRING; break;
                default:
                    errorAt(expr->as.arrayIndex.array->as.variable.name, "Cannot index a non-array type.");
                    expr->dataType = TYPE_UNKNOWN;
            }
            break;
        }
    }
}

static void analyzeStmt(Stmt* stmt) {
    if (!stmt) return;
    
    switch (stmt->type) {
        case STMT_EXPR:
            analyzeExpr(stmt->as.expr);
            break;
        case STMT_IF:
            analyzeExpr(stmt->as.ifStmt.condition);
            analyzeStmt(stmt->as.ifStmt.thenBranch);
            if (stmt->as.ifStmt.elseBranch) {
                analyzeStmt(stmt->as.ifStmt.elseBranch);
            }
            break;
        case STMT_WHILE:
            analyzeExpr(stmt->as.whileStmt.condition);
            analyzeStmt(stmt->as.whileStmt.body);
            break;
        case STMT_RETURN:
            if (stmt->as.returnExpr) {
                analyzeExpr(stmt->as.returnExpr);
                if (currentScope->currentFunctionReturnType == TYPE_VOID) {
                    // Could add a token here to give better error pos
                    fprintf(stderr, "Semantic Error: Void function cannot return a value.\n");
                    semaError = 1;
                }
            } else {
                if (currentScope->currentFunctionReturnType != TYPE_VOID) {
                    fprintf(stderr, "Semantic Error: Function must return a value.\n");
                    semaError = 1;
                }
            }
            break;
        case STMT_LET:
            analyzeExpr(stmt->as.letStmt.initializer);
            defineSymbol(stmt->as.letStmt.name, stmt->as.letStmt.declaredType, 0);
            break;
        case STMT_ASSIGN: {
            Symbol* sym = resolveSymbol(stmt->as.assignStmt.name);
            if (!sym) {
                errorAt(stmt->as.assignStmt.name, "Undeclared variable.");
            } else if (sym->isFunction) {
                errorAt(stmt->as.assignStmt.name, "Cannot assign to a function.");
            } else {
                analyzeExpr(stmt->as.assignStmt.value);
                // Type check assignment (simplified)
            }
            break;
        }
        case STMT_ASSIGN_INDEX: {
            analyzeExpr(stmt->as.assignIndexStmt.array);
            analyzeExpr(stmt->as.assignIndexStmt.index);
            analyzeExpr(stmt->as.assignIndexStmt.value);
            break;
        }
        case STMT_BLOCK:
            pushScope(TYPE_UNKNOWN);
            for (int i = 0; i < stmt->as.block.stmtCount; i++) {
                analyzeStmt(stmt->as.block.statements[i]);
            }
            popScope();
            break;
    }
}

int analyze(Program* program) {
    semaError = 0;
    pushScope(TYPE_UNKNOWN); // Global scope

    // Register built-in standard library functions
    Token tPrintInt = {TOKEN_IDENTIFIER, "print_int", 9, 0};
    defineSymbol(tPrintInt, TYPE_VOID, 1);
    
    Token tPrintString = {TOKEN_IDENTIFIER, "print_string", 12, 0};
    defineSymbol(tPrintString, TYPE_VOID, 1);
    
    Token tInputInt = {TOKEN_IDENTIFIER, "input_int", 9, 0};
    defineSymbol(tInputInt, TYPE_INT, 1);
    
    Token tInputString = {TOKEN_IDENTIFIER, "input_string", 12, 0};
    defineSymbol(tInputString, TYPE_STRING, 1);

    Token tStrlen = {TOKEN_IDENTIFIER, "strlen", 6, 0};
    defineSymbol(tStrlen, TYPE_INT, 1);

    Token tStrcmp = {TOKEN_IDENTIFIER, "strcmp", 6, 0};
    defineSymbol(tStrcmp, TYPE_INT, 1);

    Token tAllocIntArr = {TOKEN_IDENTIFIER, "allocate_int_array", 18, 0};
    defineSymbol(tAllocIntArr, TYPE_INT_ARRAY, 1);

    Token tAllocStrArr = {TOKEN_IDENTIFIER, "allocate_string_array", 21, 0};
    defineSymbol(tAllocStrArr, TYPE_STRING_ARRAY, 1);

    // First pass: Declare all functions to allow mutual recursion
    for (int i = 0; i < program->declCount; i++) {
        Decl* decl = program->declarations[i];
        if (decl->type == DECL_FUNCTION) {
            defineSymbol(decl->as.function.name, decl->as.function.returnType, 1);
        }
    }

    // Second pass: Analyze bodies
    for (int i = 0; i < program->declCount; i++) {
        Decl* decl = program->declarations[i];
        if (decl->type == DECL_FUNCTION) {
            pushScope(decl->as.function.returnType);
            for (int p = 0; p < decl->as.function.paramCount; p++) {
                defineSymbol(decl->as.function.params[p].name, decl->as.function.params[p].type, 0);
            }
            analyzeStmt(decl->as.function.body);
            popScope();
        }
    }

    popScope();
    return !semaError;
}
