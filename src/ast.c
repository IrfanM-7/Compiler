#include <stdlib.h>
#include "ast.h"

Expr* makeBinaryExpr(Expr* left, Token op, Expr* right) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_BINARY;
    expr->dataType = TYPE_UNKNOWN;
    expr->as.binary.left = left;
    expr->as.binary.op = op;
    expr->as.binary.right = right;
    return expr;
}

Expr* makeIntLiteralExpr(int value) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL_INT;
    expr->dataType = TYPE_INT;
    expr->as.intValue = value;
    return expr;
}

Expr* makeFloatLiteralExpr(float value) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL_FLOAT;
    expr->dataType = TYPE_FLOAT;
    expr->as.floatValue = value;
    return expr;
}

Expr* makeStringLiteralExpr(Token value) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL_STRING;
    expr->dataType = TYPE_STRING;
    expr->as.stringValue.value = value;
    return expr;
}

Expr* makeVariableExpr(Token name) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_VARIABLE;
    expr->dataType = TYPE_UNKNOWN;
    expr->as.variable.name = name;
    return expr;
}

Expr* makeCallExpr(Token callee, Expr** arguments, int argCount) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_CALL;
    expr->dataType = TYPE_UNKNOWN;
    expr->as.call.callee = callee;
    expr->as.call.arguments = arguments;
    expr->as.call.argCount = argCount;
    return expr;
}

Expr* makeArrayIndexExpr(Expr* array, Expr* index) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_ARRAY_INDEX;
    expr->dataType = TYPE_UNKNOWN;
    expr->as.arrayIndex.array = array;
    expr->as.arrayIndex.index = index;
    return expr;
}

Stmt* makeExprStmt(Expr* expr) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_EXPR;
    stmt->as.expr = expr;
    return stmt;
}

Stmt* makeIfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_IF;
    stmt->as.ifStmt.condition = condition;
    stmt->as.ifStmt.thenBranch = thenBranch;
    stmt->as.ifStmt.elseBranch = elseBranch;
    return stmt;
}

Stmt* makeWhileStmt(Expr* condition, Stmt* body) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_WHILE;
    stmt->as.whileStmt.condition = condition;
    stmt->as.whileStmt.body = body;
    return stmt;
}

Stmt* makeReturnStmt(Expr* expr) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_RETURN;
    stmt->as.returnExpr = expr;
    return stmt;
}

Stmt* makeLetStmt(Token name, DataType declaredType, Expr* initializer) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_LET;
    stmt->as.letStmt.name = name;
    stmt->as.letStmt.declaredType = declaredType;
    stmt->as.letStmt.initializer = initializer;
    return stmt;
}

Stmt* makeAssignStmt(Token name, Expr* value) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_ASSIGN;
    stmt->as.assignStmt.name = name;
    stmt->as.assignStmt.value = value;
    return stmt;
}

Stmt* makeAssignIndexStmt(Expr* array, Expr* index, Expr* value) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_ASSIGN_INDEX;
    stmt->as.assignIndexStmt.array = array;
    stmt->as.assignIndexStmt.index = index;
    stmt->as.assignIndexStmt.value = value;
    return stmt;
}

Stmt* makeBlockStmt(Stmt** statements, int stmtCount) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_BLOCK;
    stmt->as.block.statements = statements;
    stmt->as.block.stmtCount = stmtCount;
    return stmt;
}

Decl* makeFunctionDecl(Token name, Param* params, int paramCount, DataType returnType, Stmt* body) {
    Decl* decl = (Decl*)malloc(sizeof(Decl));
    decl->type = DECL_FUNCTION;
    decl->as.function.name = name;
    decl->as.function.params = params;
    decl->as.function.paramCount = paramCount;
    decl->as.function.returnType = returnType;
    decl->as.function.body = body;
    return decl;
}

Program* makeProgram(Decl** declarations, int declCount) {
    Program* prog = (Program*)malloc(sizeof(Program));
    prog->declarations = declarations;
    prog->declCount = declCount;
    return prog;
}
