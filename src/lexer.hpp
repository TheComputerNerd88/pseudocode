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

// --- Lexer Implementation ---

class Lexer {
private:
    std::string source;
    std::vector<Token> tokens;
    int start;
    size_t current;
    int line;
    std::unordered_map<std::string, TokenType> keywords;

    ErrorReporter reporter;
    void reportError(ErrorType type, const std::string& message) {
        // Locate start and end of current line to print a better error message.
        size_t lineStart = start;
        while (lineStart > 0 && source[lineStart - 1] != '\n') {
            lineStart--;
        }

        size_t lineEnd = current;
        while (lineEnd < source.length() && source[lineEnd] != '\n') {
            lineEnd++;
        }

        // Extract the actual line string
        std::string lineStr = source.substr(lineStart, lineEnd - lineStart);

        // Calculate the column
        size_t column = start - lineStart;

        // Pass to reporter
        reporter.report(type, line, column, message, lineStr);
    }

public:
    Lexer(const std::string& src, ErrorReporter& errReporter) :
        source(src), start(0), current(0), line(1), reporter(errReporter) {
        // --- KEYWORD MAPPING ---
        keywords["CLASS"] = TOK_CLASS;
        keywords["ATTRIBUTES"] = TOK_ATTRIBUTES;
        keywords["METHODS"] = TOK_METHODS;
        keywords["FUNCTION"] = TOK_FUNCTION;
        keywords["RETURN"] = TOK_RETURN;
        keywords["END"] = TOK_END;
        keywords["NEW"] = TOK_NEW;
        keywords["PRINT"] = TOK_PRINT;

        keywords["WHILE"] = TOK_WHILE;
        keywords["IF"] = TOK_IF;
        keywords["THEN"] = TOK_THEN;
        keywords["FOR"] = TOK_FOR;

        keywords["TRUE"] = TOK_TRUE;
        keywords["FALSE"] = TOK_FALSE;

        keywords["Attributes"] = TOK_ATTRIBUTES;
        keywords["Methods"] = TOK_METHODS;
        keywords["True"] = TOK_TRUE;
        keywords["False"] = TOK_FALSE;
        keywords["new"] = TOK_NEW;
    }

    std::vector<Token> scanTokens() {
        while (!isAtEnd()) {
            start = current;
            scanToken();
        }
        tokens.push_back({TOK_EOF, "", line});
        return tokens;
    }

private:
    bool isAtEnd() {
        return current >= source.length();
    }

    char advance() {
        return source[current++];
    }

    char peek() {
        if (isAtEnd()) return '\0';
        return source[current];
    }

    bool match(char expected) {
        if (isAtEnd()) return false;
        if (source[current] != expected) return false;
        current++;
        return true;
    }

    void addToken(TokenType type) {
        std::string text = source.substr(start, current - start);
        tokens.push_back({type, text, line});
    }

    void addToken(TokenType type, std::string literal) {
        tokens.push_back({type, literal, line});
    }

    void string(char quoteType) {
        while (peek() != quoteType && !isAtEnd()) {
            if (peek() == '\n') line++;
            advance();
        }

        if (isAtEnd()) {
            // Set current to point to where the string should end
            reportError(
                ErrorType::Syntax,
                "Unterminated string."
            );
            return;
        }

        advance();
        std::string value = source.substr(start + 1, current - start - 2);
        addToken(TOK_STRING, value);
    }

    void number() {
        // Consume all digits
        while (isdigit(peek())) {
            advance();
        }

        // Float detection, if next token is '.' proceeded with some more number
        // looking stuff then it's probably a float.
        if (peek() == '.' && isdigit(source[current + 1])) {
            advance();
            while (isdigit(peek())) advance();
            addToken(TOK_FLOAT);
        } else {
            // Add integer if it's not secretly an malformed variable name
            if (!isalpha(peek())) {
                addToken(TOK_INTEGER);
            } else {
                reportError(ErrorType::Syntax, "Identifier starts with number.");
            }
        }
    }

    void identifier() {
        // Allow alphanumeric and underscores
        while (isalnum(peek()) || peek() == '_') advance();

        std::string text = source.substr(start, current - start);
        TokenType type = TOK_IDENTIFIER;
        
        if (keywords.find(text) != keywords.end()) {
            type = keywords[text];
        }
        addToken(type);
    }

    void scanToken() {
        char c = advance();
        switch (c) {
            /**
             * Parentheses
             */
            case '(': addToken(TOK_LPAREN); break;
            case ')': addToken(TOK_RPAREN); break;
            case '[': addToken(TOK_LBRACKET); break;
            case ']': addToken(TOK_RBRACKET); break;

            /**
             * Special tokens
             */
            case ',': addToken(TOK_COMMA); break;
            case '.': addToken(TOK_DOT); break;
            case ':': addToken(TOK_COLON); break;

            /**
             * Arithmetic tokens
             */
            case '+': addToken(TOK_PLUS); break;
            case '-': addToken(TOK_MINUS); break;
            case '/':
                // Division and comments are handled together
                if (match('/')) {
                    // A comment goes until the end of the line.
                    // We keep advancing until we hit a newline or EOF.
                    // Also don't consume \n since that's consumed elsewhere.
                    while (peek() != '\n' && !isAtEnd()) {
                        advance();
                    }
                } else {
                    addToken(TOK_DIVIDE);
                }
                break;
            case '*': addToken(TOK_MULTIPLY); break;

            case '#':
                // Other method for comments
                while (peek() != '\n' && !isAtEnd()) {
                    advance();
                }
                break;

            
            /**
             * Assignment and comparisons
             */
            case '=':
                // Equality and assignments handled together
                addToken(match('=') ? TOK_EQUAL : TOK_ASSIGN);
                break;
            case '<':
                // < and <= handled together
                addToken(match('=') ? TOK_LT_OR_EQ : TOK_LESS_THAN);
                break;
            case '>':
                // > and >= handled together
                addToken(match('=') ? TOK_GT_OR_EQ : TOK_GREATER_THAN);
                break;
            
            /**
             * Skipped bytes
             */
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                line++;
                break;
            
            /**
             * String handling
             */
            case '"': string('"'); break;
            case '\'': string('\''); break;

            default:
                if (isdigit(c)) {
                    // Ints e.g. 123
                    number();
                } else if (isalpha(c) || c == '_') {
                    // Identifiers
                    identifier();
                } else {
                    // Bad character
                    std::string msg = "Unexpected character '";
                    msg += c;
                    msg += "'.";
                    reportError(ErrorType::Syntax, msg);
                }
                break;
        }
    }
};
