#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>
#include <iomanip>

#include "errors.hpp"

// --- Token Definitions ---

enum TokenType {
    // End of file
    TOK_EOF,
    
    // Literals & Identifiers
    TOK_IDENTIFIER,
    TOK_STRING,
    TOK_INTEGER,
    TOK_FLOAT,
    TOK_TRUE, 
    TOK_FALSE,
    
    // === Keywords ===
    TOK_CLASS, TOK_INHERITS,
    TOK_ATTRIBUTES, TOK_METHODS,
    TOK_FUNCTION, TOK_RETURN,
    TOK_NEW, TOK_END,
    TOK_IF, TOK_THEN, TOK_ELSE,
    TOK_WHILE, TOK_FOR,
    TOK_PRINT,
    
    // === Operators ===
    // Normal operators
    TOK_ASSIGN,         // =
    TOK_PLUS,           // +
    TOK_MINUS,          // -
    TOK_MULTIPLY,       // *
    TOK_DIVIDE,         // /

    // Comparison operators
    TOK_EQUAL,          // ==
    TOK_GREATER_THAN,   // >
    TOK_GT_OR_EQ,       // >=
    TOK_LESS_THAN,      // <
    TOK_LT_OR_EQ,       // <=

    // Special operators
    TOK_DOT,            // .
    TOK_COLON,          // :
    TOK_COMMA,          // ,

    // Parentheses
    TOK_LPAREN,         // (
    TOK_RPAREN,         // )
    TOK_LBRACKET,       // [
    TOK_RBRACKET        // ]
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;

    // For debugging - tokens can be converted to strings
    std::string typeToString() const {
        switch (type) {
            case TOK_EOF: return "EOF";
            case TOK_IDENTIFIER: return "IDENTIFIER";
            case TOK_STRING: return "STRING";
            case TOK_INTEGER: return "INTEGER";
            case TOK_FLOAT: return "FLOAT";
            case TOK_TRUE: return "BOOLEAN(True)";
            case TOK_FALSE: return "BOOLEAN(False)";
            
            case TOK_CLASS: return "KEYWORD(CLASS)";
            case TOK_ATTRIBUTES: return "KEYWORD(ATTRIBUTES)";
            case TOK_METHODS: return "KEYWORD(METHODS)";
            case TOK_FUNCTION: return "KEYWORD(FUNCTION)";
            case TOK_RETURN: return "KEYWORD(RETURN)";
            case TOK_NEW: return "KEYWORD(NEW)";
            case TOK_IF: return "KEYWORD(IF)";
            case TOK_END: return "KEYWORD(END)";
            case TOK_THEN: return "KEYWORD(THEN)";
            case TOK_WHILE: return "KEYWORD(WHILE)";
            case TOK_FOR: return "KEYWORD(FOR)";
            case TOK_PRINT: return "KEYWORD(PRINT)";
            
            case TOK_ASSIGN: return "OPERATOR(=)";

            case TOK_PLUS: return "OPERATOR(+)";
            case TOK_MINUS: return "OPERATOR(-)";
            case TOK_MULTIPLY: return "OPERATOR(*)";
            case TOK_DIVIDE: return "OPERATOR(/)";

            case TOK_EQUAL: return "OPERATOR(==)";
            case TOK_GREATER_THAN: return "OPERATOR(>)";
            case TOK_GT_OR_EQ: return "OPERATOR(>=)";
            case TOK_LESS_THAN: return "OPERATOR(<)";
            case TOK_LT_OR_EQ: return "OPERATOR(<=)";

            case TOK_DOT: return "OPERATOR(.)";
            case TOK_COLON: return "OPERATOR(:)";
            case TOK_COMMA: return "OPERATOR(,)";
            case TOK_LPAREN: return "LPAREN";
            case TOK_RPAREN: return "RPAREN";
            case TOK_LBRACKET: return "LBRACKET";
            case TOK_RBRACKET: return "RBRACKET";
            default: return "UNKNOWN";
        }
    }
};

// --- Lexer Declaration ---

class Lexer {
private:
    std::string source;
    std::vector<Token> tokens;
    int start;
    size_t current;
    int line;
    std::unordered_map<std::string, TokenType> keywords;

    ErrorReporter reporter;

public:
    Lexer(const std::string& src, ErrorReporter& errReporter);

    std::vector<Token> scanTokens();

private:
    bool isAtEnd();
    char advance();
    char peek();
    bool match(char expected);
    void addToken(TokenType type);
    void addToken(TokenType type, std::string literal);
    void reportError(ErrorType type, const std::string& message);
    void string(char quoteType);
    void number();
    void identifier();
    void scanToken();
};
