#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

static Scanner scanner;

void initLexer(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static int isAtEnd() {
    return *scanner.current == '\0';
}

static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

static int match(char expected) {
    if (isAtEnd()) return 0;
    if (*scanner.current != expected) return 0;
    scanner.current++;
    return 1;
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

static void skipWhitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                scanner.line++;
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
    switch (scanner.start[0]) {
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'n': return checkKeyword(2, 0, "", TOKEN_FN);
                    case 'l': return checkKeyword(2, 3, "oat", TOKEN_FLOAT);
                }
            }
            break;
        case 'i':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'f': return checkKeyword(2, 0, "", TOKEN_IF);
                    case 'n': return checkKeyword(2, 1, "t", TOKEN_INT);
                }
            }
            break;
        case 'l': return checkKeyword(1, 2, "et", TOKEN_LET);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 'v': return checkKeyword(1, 3, "oid", TOKEN_VOID);
        case 's': return checkKeyword(1, 5, "tring", TOKEN_STRING);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    while (isalpha(peek()) || isdigit(peek()) || peek() == '_') advance();
    return makeToken(identifierType());
}

static Token number() {
    while (isdigit(peek())) advance();
    
    if (peek() == '.' && isdigit(peekNext())) {
        advance(); // Consume the "."
        while (isdigit(peek())) advance();
        return makeToken(TOKEN_FLOAT_LITERAL);
    }
    
    return makeToken(TOKEN_INT_LITERAL);
}

static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }
    
    if (isAtEnd()) return errorToken("Unterminated string.");
    
    advance(); // The closing "
    return makeToken(TOKEN_STRING_LITERAL);
}

Token scanToken() {
    skipWhitespace();
    scanner.start = scanner.current;
    
    if (isAtEnd()) return makeToken(TOKEN_EOF);
    
    char c = advance();
    
    if (isalpha(c) || c == '_') return identifier();
    if (isdigit(c)) return number();
    
    switch (c) {
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ':': return makeToken(TOKEN_COLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '[': return makeToken(TOKEN_LEFT_BRACKET);
        case ']': return makeToken(TOKEN_RIGHT_BRACKET);
        case '"': return string();
        case '-': 
            if (match('>')) return makeToken(TOKEN_ARROW);
            return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        case '!': return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<': return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    }
    
    return errorToken("Unexpected character.");
}
