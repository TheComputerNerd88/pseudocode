#include "lexer.hpp"

Lexer::Lexer(const std::string& src, ErrorReporter& errReporter) :
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

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }
    tokens.push_back({TOK_EOF, "", line});
    return tokens;
}

bool Lexer::isAtEnd() {
    return current >= source.length();
}

char Lexer::advance() {
    return source[current++];
}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return source[current];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

void Lexer::addToken(TokenType type) {
    std::string text = source.substr(start, current - start);
    tokens.push_back({type, text, line});
}

void Lexer::addToken(TokenType type, std::string literal) {
    tokens.push_back({type, literal, line});
}

void Lexer::reportError(ErrorType type, const std::string& message) {
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

void Lexer::string(char quoteType) {
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

void Lexer::number() {
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

void Lexer::identifier() {
    // Allow alphanumeric and underscores
    while (isalnum(peek()) || peek() == '_') advance();

    std::string text = source.substr(start, current - start);
    TokenType type = TOK_IDENTIFIER;
    
    if (keywords.find(text) != keywords.end()) {
        type = keywords[text];
    }
    addToken(type);
}

void Lexer::scanToken() {
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
