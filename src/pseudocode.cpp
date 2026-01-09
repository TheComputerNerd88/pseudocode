#include "pseudocode.hpp"
#include "ast_printer.hpp"

/**
 * Read the entire contents of a file into a string
 * @param path The path to the file to read
 * @return The complete file contents as a string
 * @throws std::runtime_error if the file cannot be opened
 */
std::string Pseudocode::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/**
 * Print a formatted table of tokens to stdout
 * Displays token type, lexeme, and line number for debugging
 * @param tokens The vector of tokens to display
 */
void Pseudocode::printTokenTable(const std::vector<Token>& tokens) {
    // Print table header with column alignment
    std::cout << std::left << std::setw(20) << "TOKEN TYPE"
              << std::setw(25) << "LEXEME"
              << "LINE" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    // Print each token as a table row
    for (const Token& token : tokens) {
        if (token.type == TOK_EOF) break;  // Don't display EOF token
        std::cout << std::left << std::setw(20) << token.typeToString()
                  << std::setw(25)
                  << (token.lexeme.empty() ? "N/A" : token.lexeme)
                  << token.line << std::endl;
    }
}

/**
 * Run the interpreter on a file
 * Reads the file, tokenizes it, and displays the token table
 * @param path Path to the pseudocode file to execute
 * @return 0 on success, 1 on error
 */
int Pseudocode::runFile(const std::string& path) {
    try {
        std::string source = readFile(path);

        // Initialize error reporting at the lexing stage
        InterpreterStage stage = InterpreterStage::Lexing;
        ErrorReporter reporter(stage);
        Lexer lexer(source, reporter);

        // Tokenize the source code
        std::vector<Token> tokens = lexer.scanTokens();
        printTokenTable(tokens);

        // Parse tokens
        stage = InterpreterStage::Parsing;
        Parser parser(tokens, source, reporter);

        ASTPrinter printer;
        printer.print(parser.parse());
    } catch (const std::exception& e) {
        return 1;
    }

    return 0;
}

/**
 * Run an interactive REPL (Read-Eval-Print-Loop)
 * Allows users to input pseudocode lines interactively
 * Each line is tokenized and displayed as a table
 * @return Always returns 0
 */
int Pseudocode::runRepl() {
    InterpreterStage stage = InterpreterStage::Lexing;

    std::string line;
    while (true) {
        // Display prompt and read a line of input
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        try {
            // Tokenize the input line
            stage = InterpreterStage::Lexing;
            ErrorReporter reporter(stage);
            Lexer lexer(line, reporter);
            std::vector<Token> tokens = lexer.scanTokens();
            // printTokenTable(tokens);

            // Parse tokens
            stage = InterpreterStage::Parsing;
            Parser parser(tokens, line, reporter);
            auto statements = parser.parse();

        } catch (const std::exception& e) {
            // Display error but continue REPL
            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}
