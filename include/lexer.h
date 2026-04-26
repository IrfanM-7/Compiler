#ifndef NOVA_LEXER_H
#define NOVA_LEXER_H

typedef enum {
    // Single-character tokens
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR, TOKEN_COLON,
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,

    // One or two character tokens
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    TOKEN_ARROW, // ->

    // Literals
    TOKEN_IDENTIFIER, TOKEN_INT_LITERAL, TOKEN_FLOAT_LITERAL, TOKEN_STRING_LITERAL,

    // Keywords
    TOKEN_FN, TOKEN_RETURN, TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE, TOKEN_LET,
    TOKEN_INT, TOKEN_FLOAT, TOKEN_VOID, TOKEN_STRING,

    TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

void initLexer(const char* source);
Token scanToken(void);

#endif
