#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

typedef struct {
    Token current;
    Token previous;
    int hadError;
    int panicMode;
} Parser;

static Parser parser;

static void errorAt(Token* token, const char* message) {
    if (parser.panicMode) return;
    parser.panicMode = 1;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = 1;
}

static void error(const char* message) {
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;
        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }
    errorAtCurrent(message);
}

static int check(TokenType type) {
    return parser.current.type == type;
}

static int match(TokenType type) {
    if (!check(type)) return 0;
    advance();
    return 1;
}

// Forward declarations
static Expr* expression();
static Stmt* statement();
static Decl* declaration();
static DataType parseType();

static Expr* primary() {
    if (match(TOKEN_INT_LITERAL)) {
        int value = 0;
        sscanf(parser.previous.start, "%d", &value);
        return makeIntLiteralExpr(value);
    }
    if (match(TOKEN_FLOAT_LITERAL)) {
        float value = 0.0f;
        sscanf(parser.previous.start, "%f", &value);
        return makeFloatLiteralExpr(value);
    }
    if (match(TOKEN_STRING_LITERAL)) {
        return makeStringLiteralExpr(parser.previous);
    }
    if (match(TOKEN_IDENTIFIER)) {
        Token name = parser.previous;
        if (match(TOKEN_LEFT_PAREN)) {
            // Function call
            Expr** args = NULL;
            int argCount = 0;
            if (!check(TOKEN_RIGHT_PAREN)) {
                do {
                    args = (Expr**)realloc(args, sizeof(Expr*) * (argCount + 1));
                    args[argCount++] = expression();
                } while (match(TOKEN_COMMA));
            }
            consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
            return makeCallExpr(name, args, argCount);
        }
        
        Expr* expr = makeVariableExpr(name);
        // Postfix array indexing
        while (match(TOKEN_LEFT_BRACKET)) {
            Expr* index = expression();
            consume(TOKEN_RIGHT_BRACKET, "Expect ']' after index.");
            expr = makeArrayIndexExpr(expr, index);
        }
        return expr;
    }
    if (match(TOKEN_LEFT_PAREN)) {
        Expr* expr = expression();
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

    errorAtCurrent("Expect expression.");
    return NULL;
}

static Expr* unary() {
    // We could add unary -, ! here. Skipping for simplicity if not in spec,
    // though - might be useful.
    return primary();
}

static Expr* factor() {
    Expr* expr = unary();
    while (match(TOKEN_STAR) || match(TOKEN_SLASH)) {
        Token op = parser.previous;
        Expr* right = unary();
        expr = makeBinaryExpr(expr, op, right);
    }
    return expr;
}

static Expr* term() {
    Expr* expr = factor();
    while (match(TOKEN_PLUS) || match(TOKEN_MINUS)) {
        Token op = parser.previous;
        Expr* right = factor();
        expr = makeBinaryExpr(expr, op, right);
    }
    return expr;
}

static Expr* comparison() {
    Expr* expr = term();
    while (match(TOKEN_GREATER) || match(TOKEN_GREATER_EQUAL) ||
           match(TOKEN_LESS) || match(TOKEN_LESS_EQUAL)) {
        Token op = parser.previous;
        Expr* right = term();
        expr = makeBinaryExpr(expr, op, right);
    }
    return expr;
}

static Expr* equality() {
    Expr* expr = comparison();
    while (match(TOKEN_BANG_EQUAL) || match(TOKEN_EQUAL_EQUAL)) {
        Token op = parser.previous;
        Expr* right = comparison();
        expr = makeBinaryExpr(expr, op, right);
    }
    return expr;
}

static Expr* expression() {
    return equality();
}

static Stmt* block() {
    Stmt** statements = NULL;
    int stmtCount = 0;

    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        statements = realloc(statements, sizeof(Stmt*) * (stmtCount + 1));
        statements[stmtCount++] = statement();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
    return makeBlockStmt(statements, stmtCount);
}

static Stmt* statement() {
    if (match(TOKEN_LET)) {
        consume(TOKEN_IDENTIFIER, "Expect variable name.");
        Token name = parser.previous;
        
        consume(TOKEN_COLON, "Expect ':' after variable name.");
        DataType type = parseType();

        consume(TOKEN_EQUAL, "Expect '=' after variable type.");
        Expr* initializer = expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
        return makeLetStmt(name, type, initializer);
    }
    if (match(TOKEN_IF)) {
        consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
        Expr* condition = expression();
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
        
        consume(TOKEN_LEFT_BRACE, "Expect '{' before if body.");
        Stmt* thenBranch = block();
        Stmt* elseBranch = NULL;
        
        if (match(TOKEN_ELSE)) {
            if (match(TOKEN_IF)) {
                // else if
                // Need to unget or handle recursive descent
                // For simplicity, let's just support 'else { block }'
                // Real implementation would handle 'else if' properly
                errorAtCurrent("'else if' not supported yet. Use 'else { if ... }'");
            } else {
                consume(TOKEN_LEFT_BRACE, "Expect '{' before else body.");
                elseBranch = block();
            }
        }
        return makeIfStmt(condition, thenBranch, elseBranch);
    }
    if (match(TOKEN_WHILE)) {
        consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
        Expr* condition = expression();
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
        
        consume(TOKEN_LEFT_BRACE, "Expect '{' before while body.");
        Stmt* body = block();
        return makeWhileStmt(condition, body);
    }
    if (match(TOKEN_RETURN)) {
        Expr* expr = NULL;
        if (!check(TOKEN_SEMICOLON)) {
            expr = expression();
        }
        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        return makeReturnStmt(expr);
    }
    if (match(TOKEN_LEFT_BRACE)) {
        return block();
    }

    // Assignment or expression statement
    Expr* expr = expression();
    
    if (match(TOKEN_EQUAL)) {
        // We need to check if the left hand side was an assignable expression (variable or index)
        if (expr->type == EXPR_VARIABLE) {
            Token name = expr->as.variable.name;
            Expr* value = expression();
            consume(TOKEN_SEMICOLON, "Expect ';' after assignment.");
            return makeAssignStmt(name, value);
        } else if (expr->type == EXPR_ARRAY_INDEX) {
            Expr* array = expr->as.arrayIndex.array;
            Expr* index = expr->as.arrayIndex.index;
            Expr* value = expression();
            consume(TOKEN_SEMICOLON, "Expect ';' after assignment.");
            return makeAssignIndexStmt(array, index, value);
        } else {
            error("Invalid assignment target.");
        }
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    return makeExprStmt(expr);
}

static DataType parseType() {
    DataType type = TYPE_UNKNOWN;
    if (match(TOKEN_INT)) type = TYPE_INT;
    else if (match(TOKEN_FLOAT)) type = TYPE_FLOAT;
    else if (match(TOKEN_STRING)) type = TYPE_STRING;
    else if (match(TOKEN_VOID)) type = TYPE_VOID;
    else errorAtCurrent("Expect type name.");

    // Check for array types like int[]
    if (match(TOKEN_LEFT_BRACKET)) {
        consume(TOKEN_RIGHT_BRACKET, "Expect ']' after '[' in array type.");
        if (type == TYPE_INT) type = TYPE_INT_ARRAY;
        else if (type == TYPE_FLOAT) type = TYPE_FLOAT_ARRAY;
        else if (type == TYPE_STRING) type = TYPE_STRING_ARRAY;
        else error("Cannot have an array of void.");
    }
    
    return type;
}

static Decl* declaration() {
    if (match(TOKEN_FN)) {
        consume(TOKEN_IDENTIFIER, "Expect function name.");
        Token name = parser.previous;
        
        consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
        Param* params = NULL;
        int paramCount = 0;
        if (!check(TOKEN_RIGHT_PAREN)) {
            do {
                consume(TOKEN_IDENTIFIER, "Expect parameter name.");
                Token paramName = parser.previous;
                consume(TOKEN_COLON, "Expect ':' after parameter name.");
                DataType paramType = parseType();
                
                params = realloc(params, sizeof(Param) * (paramCount + 1));
                params[paramCount].name = paramName;
                params[paramCount].type = paramType;
                paramCount++;
            } while (match(TOKEN_COMMA));
        }
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
        
        consume(TOKEN_ARROW, "Expect '->' before return type.");
        DataType returnType = parseType();
        
        consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
        Stmt* body = block();
        
        return makeFunctionDecl(name, params, paramCount, returnType, body);
    }

    errorAtCurrent("Expect declaration.");
    advance(); // panic recovery
    return NULL;
}

Program* parse(const char* source) {
    initLexer(source);
    parser.hadError = 0;
    parser.panicMode = 0;
    advance();

    Decl** declarations = NULL;
    int declCount = 0;

    while (!match(TOKEN_EOF)) {
        Decl* decl = declaration();
        if (decl != NULL) {
            declarations = realloc(declarations, sizeof(Decl*) * (declCount + 1));
            declarations[declCount++] = decl;
        }
        if (parser.panicMode) {
            // Synchronize
            parser.panicMode = 0;
            while (!check(TOKEN_EOF)) {
                if (parser.previous.type == TOKEN_SEMICOLON) return NULL; // try to recover, simplified
                switch (parser.current.type) {
                    case TOKEN_FN:
                    case TOKEN_LET:
                    case TOKEN_IF:
                    case TOKEN_WHILE:
                    case TOKEN_RETURN:
                        goto sync_done;
                    default:
                        ; // Do nothing.
                }
                advance();
            }
        sync_done:
            ;
        }
    }

    if (parser.hadError) return NULL;
    return makeProgram(declarations, declCount);
}
