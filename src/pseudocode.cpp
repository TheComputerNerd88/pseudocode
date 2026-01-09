#include "pseudocode.hpp"

std::string Pseudocode::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Pseudocode::printTokenTable(const std::vector<Token>& tokens) {
    std::cout << std::left << std::setw(20) << "TOKEN TYPE"
              << std::setw(25) << "LEXEME"
              << "LINE" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for (const Token& token : tokens) {
        if (token.type == TOK_EOF) break;
        std::cout << std::left << std::setw(20) << token.typeToString()
                  << std::setw(25)
                  << (token.lexeme.empty() ? "N/A" : token.lexeme)
                  << token.line << std::endl;
    }
}

int Pseudocode::runFile(const std::string& path) {
    try {
        std::string source = readFile(path);

        InterpreterStage stage = InterpreterStage::Lexing;
        ErrorReporter reporter(stage);
        Lexer lexer(source, reporter);

        std::vector<Token> tokens = lexer.scanTokens();
        printTokenTable(tokens);
    } catch (const std::exception& e) {
        return 1;
    }

    return 0;
}

int Pseudocode::runRepl() {
    InterpreterStage stage = InterpreterStage::Lexing;

    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        try {
            ErrorReporter reporter(stage);
            Lexer lexer(line, reporter);
            std::vector<Token> tokens = lexer.scanTokens();
            printTokenTable(tokens);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}
