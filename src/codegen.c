#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

static FILE* out = NULL;
static int tempVarCount = 1;
static int labelCount = 1;

static int genTemp() {
    return tempVarCount++;
}

static int genLabel() {
    return labelCount++;
}

static const char* llvmType(DataType type) {
    switch (type) {
        case TYPE_INT: return "i32";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "i8*";
        case TYPE_INT_ARRAY: return "i32*";
        case TYPE_FLOAT_ARRAY: return "float*";
        case TYPE_STRING_ARRAY: return "i8**";
        case TYPE_VOID: return "void";
        default: return "i32";
    }
}

// Simple environment to keep track of variable alloca pointers
typedef struct EnvNode {
    const char* name;
    int length;
    int ptrVar; // e.g. %1, %2
    struct EnvNode* next;
} EnvNode;

static EnvNode* env = NULL;

static void pushVar(Token name, int ptrVar) {
    EnvNode* node = (EnvNode*)malloc(sizeof(EnvNode));
    node->name = name.start;
    node->length = name.length;
    node->ptrVar = ptrVar;
    node->next = env;
    env = node;
}

static int getVarPtr(Token name) {
    EnvNode* curr = env;
    while (curr) {
        if (curr->length == name.length && memcmp(curr->name, name.start, name.length) == 0) {
            return curr->ptrVar;
        }
        curr = curr->next;
    }
    return -1; // Should not happen if Sema passed
}

static void clearEnv() {
    EnvNode* curr = env;
    while (curr) {
        EnvNode* next = curr->next;
        free(curr);
        curr = next;
    }
    env = NULL;
}

static int generateExpr(Expr* expr);
static void generateStmt(Stmt* stmt);

static int generateExpr(Expr* expr) {
    if (!expr) return 0;

    switch (expr->type) {
        case EXPR_LITERAL_INT: {
            int t = genTemp();
            // LLVM IR usually just uses the literal directly in instructions,
            // but we can generate a simple add with 0 to load it into a temp
            fprintf(out, "  %%%d = add i32 0, %d\n", t, expr->as.intValue);
            return t;
        }
        case EXPR_LITERAL_FLOAT: {
            int t = genTemp();
            fprintf(out, "  %%%d = fadd float 0.0, %f\n", t, expr->as.floatValue);
            return t;
        }
        case EXPR_LITERAL_STRING: {
            int t = genTemp();
            // In a real compiler, we'd emit a global constant. For this DSL, 
            // we'll assume the C backend/linking handles it or we use a hack.
            // For LLVM, we'll emit a comment or placeholder for now as strings are complex.
            fprintf(out, "  ; string literal: %.*s\n", expr->as.stringValue.value.length, expr->as.stringValue.value.start);
            fprintf(out, "  %%%d = add i32 0, 0 ; placeholder\n", t);
            return t;
        }
        case EXPR_VARIABLE: {
            int ptr = getVarPtr(expr->as.variable.name);
            int t = genTemp();
            fprintf(out, "  %%%d = load %s, %s* %%%d\n", t, llvmType(expr->dataType), llvmType(expr->dataType), ptr);
            return t;
        }
        case EXPR_ARRAY_INDEX: {
            int arrayPtr = generateExpr(expr->as.arrayIndex.array);
            int index = generateExpr(expr->as.arrayIndex.index);
            int t = genTemp();
            // Use GEP to get pointer to element
            fprintf(out, "  %%%d = getelementptr %s, %s %%%d, i32 %%%d\n", t, 
                    llvmType(expr->dataType), llvmType(expr->as.arrayIndex.array->dataType), arrayPtr, index);
            int res = genTemp();
            fprintf(out, "  %%%d = load %s, %s* %%%d\n", res, llvmType(expr->dataType), llvmType(expr->dataType), t);
            return res;
        }
        case EXPR_BINARY: {
            int left = generateExpr(expr->as.binary.left);
            int right = generateExpr(expr->as.binary.right);
            int t = genTemp();
            const char* typeStr = llvmType(expr->as.binary.left->dataType); // assuming both sides same type
            int isFloat = (expr->as.binary.left->dataType == TYPE_FLOAT);
            
            switch (expr->as.binary.op.type) {
                case TOKEN_PLUS:
                    fprintf(out, "  %%%d = %s %s %%%d, %%%d\n", t, isFloat ? "fadd" : "add", typeStr, left, right);
                    break;
                case TOKEN_MINUS:
                    fprintf(out, "  %%%d = %s %s %%%d, %%%d\n", t, isFloat ? "fsub" : "sub", typeStr, left, right);
                    break;
                case TOKEN_STAR:
                    fprintf(out, "  %%%d = %s %s %%%d, %%%d\n", t, isFloat ? "fmul" : "mul", typeStr, left, right);
                    break;
                case TOKEN_SLASH:
                    fprintf(out, "  %%%d = %s %s %%%d, %%%d\n", t, isFloat ? "fdiv" : "sdiv", typeStr, left, right);
                    break;
                case TOKEN_LESS:
                    fprintf(out, "  %%%d = %s %s %%%d, %%%d\n", t, isFloat ? "fcmp olt" : "icmp slt", typeStr, left, right);
                    // zext i1 to i32
                    int t2 = genTemp();
                    fprintf(out, "  %%%d = zext i1 %%%d to i32\n", t2, t);
                    return t2;
                case TOKEN_EQUAL_EQUAL:
                    fprintf(out, "  %%%d = %s %s %%%d, %%%d\n", t, isFloat ? "fcmp oeq" : "icmp eq", typeStr, left, right);
                    int t3 = genTemp();
                    fprintf(out, "  %%%d = zext i1 %%%d to i32\n", t3, t);
                    return t3;
                default:
                    // Other ops similar
                    break;
            }
            return t;
        }
        case EXPR_CALL: {
            // Evaluate arguments
            int* argTemps = (int*)malloc(sizeof(int) * expr->as.call.argCount);
            for (int i = 0; i < expr->as.call.argCount; i++) {
                argTemps[i] = generateExpr(expr->as.call.arguments[i]);
            }
            
            int t = 0;
            if (expr->dataType != TYPE_VOID) {
                t = genTemp();
                fprintf(out, "  %%%d = call %s @%.*s(", t, llvmType(expr->dataType), 
                        expr->as.call.callee.length, expr->as.call.callee.start);
            } else {
                fprintf(out, "  call void @%.*s(", expr->as.call.callee.length, expr->as.call.callee.start);
            }
            
            for (int i = 0; i < expr->as.call.argCount; i++) {
                fprintf(out, "%s %%%d%s", llvmType(expr->as.call.arguments[i]->dataType), 
                        argTemps[i], i < expr->as.call.argCount - 1 ? ", " : "");
            }
            fprintf(out, ")\n");
            free(argTemps);
            return t;
        }
    }
    return 0;
}

static void generateStmt(Stmt* stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case STMT_EXPR:
            generateExpr(stmt->as.expr);
            break;
        case STMT_LET: {
            int ptr = genTemp();
            fprintf(out, "  %%%d = alloca %s\n", ptr, llvmType(stmt->as.letStmt.declaredType));
            pushVar(stmt->as.letStmt.name, ptr);
            int val = generateExpr(stmt->as.letStmt.initializer);
            fprintf(out, "  store %s %%%d, %s* %%%d\n", 
                    llvmType(stmt->as.letStmt.declaredType), val, 
                    llvmType(stmt->as.letStmt.declaredType), ptr);
            break;
        }
        case STMT_ASSIGN: {
            int ptr = getVarPtr(stmt->as.assignStmt.name);
            int val = generateExpr(stmt->as.assignStmt.value);
            // We need to know the type. For simplicity, assume int if unknown.
            fprintf(out, "  store i32 %%%d, i32* %%%d\n", val, ptr);
            break;
        }
        case STMT_ASSIGN_INDEX: {
            int arrayPtr = generateExpr(stmt->as.assignIndexStmt.array);
            int index = generateExpr(stmt->as.assignIndexStmt.index);
            int val = generateExpr(stmt->as.assignIndexStmt.value);
            int targetPtr = genTemp();
            // GEP
            fprintf(out, "  %%%d = getelementptr i32, i32* %%%d, i32 %%%d\n", targetPtr, arrayPtr, index);
            fprintf(out, "  store i32 %%%d, i32* %%%d\n", val, targetPtr);
            break;
        }
        case STMT_RETURN: {
            if (stmt->as.returnExpr) {
                int val = generateExpr(stmt->as.returnExpr);
                fprintf(out, "  ret %s %%%d\n", llvmType(stmt->as.returnExpr->dataType), val);
            } else {
                fprintf(out, "  ret void\n");
            }
            break;
        }
        case STMT_IF: {
            int cond = generateExpr(stmt->as.ifStmt.condition);
            int condBool = genTemp();
            fprintf(out, "  %%%d = trunc i32 %%%d to i1\n", condBool, cond);
            
            int thenLabel = genLabel();
            int elseLabel = genLabel();
            int endLabel = genLabel();
            
            if (stmt->as.ifStmt.elseBranch) {
                fprintf(out, "  br i1 %%%d, label %%L%d, label %%L%d\n", condBool, thenLabel, elseLabel);
            } else {
                fprintf(out, "  br i1 %%%d, label %%L%d, label %%L%d\n", condBool, thenLabel, endLabel);
            }
            
            fprintf(out, "L%d:\n", thenLabel);
            generateStmt(stmt->as.ifStmt.thenBranch);
            fprintf(out, "  br label %%L%d\n", endLabel);
            
            if (stmt->as.ifStmt.elseBranch) {
                fprintf(out, "L%d:\n", elseLabel);
                generateStmt(stmt->as.ifStmt.elseBranch);
                fprintf(out, "  br label %%L%d\n", endLabel);
            }
            
            fprintf(out, "L%d:\n", endLabel);
            break;
        }
        case STMT_WHILE: {
            int condLabel = genLabel();
            int bodyLabel = genLabel();
            int endLabel = genLabel();
            
            fprintf(out, "  br label %%L%d\n", condLabel);
            fprintf(out, "L%d:\n", condLabel);
            
            int cond = generateExpr(stmt->as.whileStmt.condition);
            int condBool = genTemp();
            fprintf(out, "  %%%d = trunc i32 %%%d to i1\n", condBool, cond);
            fprintf(out, "  br i1 %%%d, label %%L%d, label %%L%d\n", condBool, bodyLabel, endLabel);
            
            fprintf(out, "L%d:\n", bodyLabel);
            generateStmt(stmt->as.whileStmt.body);
            fprintf(out, "  br label %%L%d\n", condLabel);
            
            fprintf(out, "L%d:\n", endLabel);
            break;
        }
        case STMT_BLOCK: {
            for (int i = 0; i < stmt->as.block.stmtCount; i++) {
                generateStmt(stmt->as.block.statements[i]);
            }
            break;
        }
    }
}

void generateCode(Program* program, const char* outputFilename) {
    out = fopen(outputFilename, "w");
    if (!out) {
        fprintf(stderr, "Could not open %s for writing.\n", outputFilename);
        return;
    }

    fprintf(out, "; ModuleID = 'Nova'\n");
    fprintf(out, "source_filename = \"Nova\"\n\n");

    for (int i = 0; i < program->declCount; i++) {
        Decl* decl = program->declarations[i];
        if (decl->type == DECL_FUNCTION) {
            tempVarCount = 1;
            clearEnv();
            
            fprintf(out, "define %s @%.*s(", llvmType(decl->as.function.returnType), 
                    decl->as.function.name.length, decl->as.function.name.start);
                    
            for (int p = 0; p < decl->as.function.paramCount; p++) {
                fprintf(out, "%s %%%.*s%s", llvmType(decl->as.function.params[p].type), 
                        decl->as.function.params[p].name.length, decl->as.function.params[p].name.start,
                        p < decl->as.function.paramCount - 1 ? ", " : "");
            }
            fprintf(out, ") {\nentry:\n");
            
            // Allocate space for params and store them
            for (int p = 0; p < decl->as.function.paramCount; p++) {
                int ptr = genTemp();
                fprintf(out, "  %%%d = alloca %s\n", ptr, llvmType(decl->as.function.params[p].type));
                fprintf(out, "  store %s %%%.*s, %s* %%%d\n", 
                        llvmType(decl->as.function.params[p].type),
                        decl->as.function.params[p].name.length, decl->as.function.params[p].name.start,
                        llvmType(decl->as.function.params[p].type), ptr);
                pushVar(decl->as.function.params[p].name, ptr);
            }
            
            generateStmt(decl->as.function.body);
            
            // Default return if omitted (just in case)
            if (decl->as.function.returnType == TYPE_VOID) {
                fprintf(out, "  ret void\n");
            } else {
                fprintf(out, "  ret %s 0\n", llvmType(decl->as.function.returnType));
            }
            
            fprintf(out, "}\n\n");
        }
    }

    fclose(out);
}

// C Codegen Fallback
static const char* cType(DataType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "char*";
        case TYPE_INT_ARRAY: return "int*";
        case TYPE_FLOAT_ARRAY: return "float*";
        case TYPE_STRING_ARRAY: return "char**";
        case TYPE_VOID: return "void";
        default: return "int";
    }
}

static void generateCExpr(FILE* f, Expr* expr) {
    if (!expr) return;
    switch (expr->type) {
        case EXPR_LITERAL_INT: fprintf(f, "%d", expr->as.intValue); break;
        case EXPR_LITERAL_FLOAT: fprintf(f, "%f", expr->as.floatValue); break;
        case EXPR_LITERAL_STRING: fprintf(f, "%.*s", expr->as.stringValue.value.length, expr->as.stringValue.value.start); break;
        case EXPR_VARIABLE: fprintf(f, "%.*s", expr->as.variable.name.length, expr->as.variable.name.start); break;
        case EXPR_BINARY:
            fprintf(f, "(");
            generateCExpr(f, expr->as.binary.left);
            fprintf(f, " %.*s ", expr->as.binary.op.length, expr->as.binary.op.start);
            generateCExpr(f, expr->as.binary.right);
            fprintf(f, ")");
            break;
        case EXPR_CALL:
            fprintf(f, "%.*s(", expr->as.call.callee.length, expr->as.call.callee.start);
            for (int i = 0; i < expr->as.call.argCount; i++) {
                generateCExpr(f, expr->as.call.arguments[i]);
                if (i < expr->as.call.argCount - 1) fprintf(f, ", ");
            }
            fprintf(f, ")");
            break;
        case EXPR_ARRAY_INDEX:
            generateCExpr(f, expr->as.arrayIndex.array);
            fprintf(f, "[");
            generateCExpr(f, expr->as.arrayIndex.index);
            fprintf(f, "]");
            break;
    }
}

static void generateCStmt(FILE* f, Stmt* stmt) {
    if (!stmt) return;
    switch (stmt->type) {
        case STMT_EXPR:
            generateCExpr(f, stmt->as.expr);
            fprintf(f, ";\n");
            break;
        case STMT_LET:
            fprintf(f, "%s %.*s = ", cType(stmt->as.letStmt.declaredType), stmt->as.letStmt.name.length, stmt->as.letStmt.name.start);
            generateCExpr(f, stmt->as.letStmt.initializer);
            fprintf(f, ";\n");
            break;
        case STMT_RETURN:
            fprintf(f, "return ");
            if (stmt->as.returnExpr) generateCExpr(f, stmt->as.returnExpr);
            fprintf(f, ";\n");
            break;
        case STMT_ASSIGN:
            fprintf(f, "%.*s = ", stmt->as.assignStmt.name.length, stmt->as.assignStmt.name.start);
            generateCExpr(f, stmt->as.assignStmt.value);
            fprintf(f, ";\n");
            break;
        case STMT_ASSIGN_INDEX:
            generateCExpr(f, stmt->as.assignIndexStmt.array);
            fprintf(f, "[");
            generateCExpr(f, stmt->as.assignIndexStmt.index);
            fprintf(f, "] = ");
            generateCExpr(f, stmt->as.assignIndexStmt.value);
            fprintf(f, ";\n");
            break;
        case STMT_IF:
            fprintf(f, "if (");
            generateCExpr(f, stmt->as.ifStmt.condition);
            fprintf(f, ") {\n");
            generateCStmt(f, stmt->as.ifStmt.thenBranch);
            fprintf(f, "}");
            if (stmt->as.ifStmt.elseBranch) {
                fprintf(f, " else {\n");
                generateCStmt(f, stmt->as.ifStmt.elseBranch);
                fprintf(f, "}\n");
            } else {
                fprintf(f, "\n");
            }
            break;
        case STMT_WHILE:
            fprintf(f, "while (");
            generateCExpr(f, stmt->as.whileStmt.condition);
            fprintf(f, ") {\n");
            generateCStmt(f, stmt->as.whileStmt.body);
            fprintf(f, "}\n");
            break;
        case STMT_BLOCK:
            for (int i = 0; i < stmt->as.block.stmtCount; i++) {
                generateCStmt(f, stmt->as.block.statements[i]);
            }
            break;
    }
}

void generateCCode(Program* program, const char* outputFilename) {
    FILE* f = fopen(outputFilename, "w");
    if (!f) return;
    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "#include <stdlib.h>\n");
    fprintf(f, "#include <string.h>\n\n");

    // Standard Library Wrappers
    fprintf(f, "void print_int(int x) { printf(\"%%d\\n\", x); }\n");
    fprintf(f, "void print_string(char* x) { printf(\"%%s\\n\", x); }\n");
    fprintf(f, "int input_int() { int x; scanf(\"%%d\", &x); return x; }\n");
    fprintf(f, "char* input_string() { char* s = malloc(256); scanf(\"%%s\", s); return s; }\n");
    fprintf(f, "int* allocate_int_array(int size) { return (int*)malloc(size * sizeof(int)); }\n");
    fprintf(f, "char** allocate_string_array(int size) { return (char**)malloc(size * sizeof(char*)); }\n\n");

    // Forward declarations
    for (int i = 0; i < program->declCount; i++) {
        Decl* decl = program->declarations[i];
        if (decl->type == DECL_FUNCTION) {
            int isMain = (decl->as.function.name.length == 4 && memcmp(decl->as.function.name.start, "main", 4) == 0);
            fprintf(f, "%s %s%.*s(", cType(decl->as.function.returnType), 
                    isMain ? "nova_" : "",
                    decl->as.function.name.length, decl->as.function.name.start);
            for (int p = 0; p < decl->as.function.paramCount; p++) {
                fprintf(f, "%s %.*s%s", cType(decl->as.function.params[p].type), decl->as.function.params[p].name.length, decl->as.function.params[p].name.start, p < decl->as.function.paramCount - 1 ? ", " : "");
            }
            fprintf(f, ");\n");
        }
    }
    fprintf(f, "\n");

    // Definitions
    for (int i = 0; i < program->declCount; i++) {
        Decl* decl = program->declarations[i];
        if (decl->type == DECL_FUNCTION) {
            int isMain = (decl->as.function.name.length == 4 && memcmp(decl->as.function.name.start, "main", 4) == 0);
            
            fprintf(f, "%s %s%s%.*s(", cType(decl->as.function.returnType), 
                    isMain ? "nova_" : "", // Prefix Nova's main
                    "", decl->as.function.name.length, decl->as.function.name.start);
            for (int p = 0; p < decl->as.function.paramCount; p++) {
                fprintf(f, "%s %.*s%s", cType(decl->as.function.params[p].type), decl->as.function.params[p].name.length, decl->as.function.params[p].name.start, p < decl->as.function.paramCount - 1 ? ", " : "");
            }
            fprintf(f, ") {\n");
            generateCStmt(f, decl->as.function.body);
            fprintf(f, "}\n\n");
        }
    }
    fprintf(f, "int main() { return nova_main(); }\n");
    fclose(f);
}
