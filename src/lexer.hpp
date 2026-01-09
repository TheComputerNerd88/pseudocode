#pragma once

#include <cctype>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

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
    TOK_CLASS,
    TOK_INHERITS,
    TOK_ATTRIBUTES,
    TOK_METHODS,
    TOK_FUNCTION,
    TOK_RETURN,
    TOK_NEW,
    TOK_END,
    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_WHILE,
    TOK_FOR,
    TOK_IN,
    TOK_PRINT,

    // === Operators ===
    // Normal operators
    TOK_ASSIGN,   // =
    TOK_PLUS,     // +
    TOK_MINUS,    // -
    TOK_MULTIPLY, // *
    TOK_DIVIDE,   // /

    // Comparison operators
    TOK_EQUAL,        // ==
    TOK_GREATER_THAN, // >
    TOK_GT_OR_EQ,     // >=
    TOK_LESS_THAN,    // <
    TOK_LT_OR_EQ,     // <=

    // Special operators
    TOK_DOT,   // .
    TOK_COLON, // :
    TOK_COMMA, // ,

    // Parentheses
    TOK_LPAREN,   // (
    TOK_RPAREN,   // )
    TOK_LBRACKET, // [
    TOK_RBRACKET  // ]
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column; // Column position (0-indexed)
    int length; // Length of the token in characters

    /**
     * Convert token type to human-readable string for debugging
     * @return String representation of the token type
     */
    std::string typeToString() const {
        switch (type) {
        case TOK_EOF:
            return "EOF";
        case TOK_IDENTIFIER:
            return "IDENTIFIER";
        case TOK_STRING:
            return "STRING";
        case TOK_INTEGER:
            return "INTEGER";
        case TOK_FLOAT:
            return "FLOAT";
        case TOK_TRUE:
            return "BOOLEAN(True)";
        case TOK_FALSE:
            return "BOOLEAN(False)";

        case TOK_CLASS:
            return "KEYWORD(CLASS)";
        case TOK_ATTRIBUTES:
            return "KEYWORD(ATTRIBUTES)";
        case TOK_METHODS:
            return "KEYWORD(METHODS)";
        case TOK_FUNCTION:
            return "KEYWORD(FUNCTION)";
        case TOK_RETURN:
            return "KEYWORD(RETURN)";
        case TOK_NEW:
            return "KEYWORD(NEW)";
        case TOK_IF:
            return "KEYWORD(IF)";
        case TOK_END:
            return "KEYWORD(END)";
        case TOK_THEN:
            return "KEYWORD(THEN)";
        case TOK_IN:
            return "KEYWORD(IN)";
        case TOK_ELSE:
            return "KEYWORD(ELSE)";
        case TOK_WHILE:
            return "KEYWORD(WHILE)";
        case TOK_FOR:
            return "KEYWORD(FOR)";
        case TOK_PRINT:
            return "KEYWORD(PRINT)";

        case TOK_ASSIGN:
            return "OPERATOR(=)";

        case TOK_PLUS:
            return "OPERATOR(+)";
        case TOK_MINUS:
            return "OPERATOR(-)";
        case TOK_MULTIPLY:
            return "OPERATOR(*)";
        case TOK_DIVIDE:
            return "OPERATOR(/)";

        case TOK_EQUAL:
            return "OPERATOR(==)";
        case TOK_GREATER_THAN:
            return "OPERATOR(>)";
        case TOK_GT_OR_EQ:
            return "OPERATOR(>=)";
        case TOK_LESS_THAN:
            return "OPERATOR(<)";
        case TOK_LT_OR_EQ:
            return "OPERATOR(<=)";

        case TOK_DOT:
            return "OPERATOR(.)";
        case TOK_COLON:
            return "OPERATOR(:)";
        case TOK_COMMA:
            return "OPERATOR(,)";
        case TOK_LPAREN:
            return "LPAREN";
        case TOK_RPAREN:
            return "RPAREN";
        case TOK_LBRACKET:
            return "LBRACKET";
        case TOK_RBRACKET:
            return "RBRACKET";
        default:
            return "UNKNOWN";
        }
    }
};

// --- Lexer Declaration ---
// The Lexer class tokenizes pseudocode source into a stream of tokens.
// It handles keywords, identifiers, literals, operators, and comments.

class Lexer {
  private:
    // Source code being tokenized
    std::string source;
    // Output tokens collected during scanning
    std::vector<Token> tokens;
    // Start position of current token in source
    int start;
    // Current position in source
    size_t current;
    // Current line number (for error reporting)
    int line;
    // Column at the start of current token (0-indexed)
    int startColumn;
    // Current column position in the current line (0-indexed)
    int column;
    // Map of keywords to their token types
    std::unordered_map<std::string, TokenType> keywords;

    // Error reporter for communicating issues
    ErrorReporter reporter;

  public:
    /**
     * Construct a lexer for the given source code
     * @param src The source code string to tokenize
     * @param errReporter Reference to error reporter
     */
    Lexer(const std::string &src, ErrorReporter &errReporter);

    /**
     * Scan all tokens from the source code
     * @return Vector of all tokens in the source
     */
    std::vector<Token> scanTokens();

  private:
    /**
     * Check if we're at the end of source
     */
    bool isAtEnd();

    /**
     * Consume and return the current character
     */
    char advance();

    /**
     * Peek at the current character without consuming
     */
    char peek();

    /**
     * Conditionally consume a character if it matches expected
     */
    bool match(char expected);

    /**
     * Add a token to the token list from current span
     */
    void addToken(TokenType type);

    /**
     * Add a token with a specific literal value
     */
    void addToken(TokenType type, std::string literal);

    /**
     * Report a lexical error with context
     */
    void reportError(ErrorType type, const std::string &message);

    /**
     * Scan a string literal
     */
    void string(char quoteType);

    /**
     * Scan a numeric literal (int or float)
     */
    void number();

    /**
     * Scan an identifier or keyword
     */
    void identifier();

    /**
     * Main token scanning dispatch
     */
    void scanToken();
};
