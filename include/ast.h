#ifndef NOVA_AST_H
#define NOVA_AST_H

#include "lexer.h"

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_INT_ARRAY,
    TYPE_FLOAT_ARRAY,
    TYPE_STRING_ARRAY,
    TYPE_VOID,
    TYPE_UNKNOWN
} DataType;

// Forward declarations
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Decl Decl;

// Expressions
typedef enum {
    EXPR_BINARY,
    EXPR_LITERAL_INT,
    EXPR_LITERAL_FLOAT,
    EXPR_LITERAL_STRING,
    EXPR_VARIABLE,
    EXPR_CALL,
    EXPR_ARRAY_INDEX
} ExprType;

struct Expr {
    ExprType type;
    DataType dataType;
    union {
        struct { Expr* left; Token op; Expr* right; } binary;
        int intValue;
        float floatValue;
        struct { Token value; } stringValue;
        struct { Token name; } variable;
        struct { Token callee; Expr** arguments; int argCount; } call;
        struct { Expr* array; Expr* index; } arrayIndex;
    } as;
};

// Statements
typedef enum {
    STMT_EXPR,
    STMT_IF,
    STMT_WHILE,
    STMT_RETURN,
    STMT_LET,
    STMT_ASSIGN,
    STMT_ASSIGN_INDEX,
    STMT_BLOCK
} StmtType;

struct Stmt {
    StmtType type;
    union {
        Expr* expr;
        struct { Expr* condition; Stmt* thenBranch; Stmt* elseBranch; } ifStmt;
        struct { Expr* condition; Stmt* body; } whileStmt;
        Expr* returnExpr; // Can be NULL for void returns
        struct { Token name; DataType declaredType; Expr* initializer; } letStmt;
        struct { Token name; Expr* value; } assignStmt;
        struct { Expr* array; Expr* index; Expr* value; } assignIndexStmt;
        struct { Stmt** statements; int stmtCount; } block;
    } as;
};

// Declarations
typedef enum {
    DECL_FUNCTION
} DeclType;

typedef struct {
    Token name;
    DataType type;
} Param;

struct Decl {
    DeclType type;
    union {
        struct {
            Token name;
            Param* params;
            int paramCount;
            DataType returnType;
            Stmt* body; // STMT_BLOCK
        } function;
    } as;
};

typedef struct {
    Decl** declarations;
    int declCount;
} Program;

// AST Constructors
Expr* makeBinaryExpr(Expr* left, Token op, Expr* right);
Expr* makeIntLiteralExpr(int value);
Expr* makeFloatLiteralExpr(float value);
Expr* makeStringLiteralExpr(Token value);
Expr* makeVariableExpr(Token name);
Expr* makeCallExpr(Token callee, Expr** arguments, int argCount);
Expr* makeArrayIndexExpr(Expr* array, Expr* index);

Stmt* makeExprStmt(Expr* expr);
Stmt* makeIfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch);
Stmt* makeWhileStmt(Expr* condition, Stmt* body);
Stmt* makeReturnStmt(Expr* expr);
Stmt* makeLetStmt(Token name, DataType declaredType, Expr* initializer);
Stmt* makeAssignStmt(Token name, Expr* value);
Stmt* makeAssignIndexStmt(Expr* array, Expr* index, Expr* value);
Stmt* makeBlockStmt(Stmt** statements, int stmtCount);

Decl* makeFunctionDecl(Token name, Param* params, int paramCount, DataType returnType, Stmt* body);
Program* makeProgram(Decl** declarations, int declCount);

#endif
