#pragma once

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "errors.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

/**
 * Main Pseudocode interpreter class
 * Provides both file execution and interactive REPL modes
 * Exposes a public API for running pseudocode programs
 */
class Pseudocode {
public:
    /**
     * Execute pseudocode from a file
     * @param path Path to the pseudocode file to run
     * @return 0 on success, 1 on error
     */
    int runFile(const std::string &path);

    /**
     * Run an interactive REPL (Read-Eval-Print-Loop)
     * Allows users to type pseudocode lines and see tokenization
     * @return Always returns 0
     */
    int runRepl();

    /**
     * Optional debug modes for debugging tokenization and parsing
     */
    bool debugTokens = false; // Print token table after Lexing
    bool debugParse  = false; // Print AST after Parsing
private:
    /**
     * Read entire file contents into a string
     * @param path Path to the file to read
     * @return File contents as string
     */
    static std::string readFile(const std::string &path);

    /**
     * Print tokens in a formatted table
     * @param tokens Vector of tokens to display
     */
    static void printTokenTable(const std::vector<Token> &tokens);
};

void help() {
    std::cout << "Usage: scsa [--debug-tokens] [--debug-parse] [script.scsa]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --debug-tokens   Print token table after lexing" << std::endl;
    std::cout << "  --debug-parse    Print AST after parsing" << std::endl;
    std::cout << "If no script is provided, an interactive REPL is started." << std::endl;
}

/**
 * Entry point for the Pseudocode interpreter
 *
 */
int main(int argc, char *argv[]) {

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(hConsole, &consoleMode);
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsole, consoleMode); // Enable ANSI colours

    // Set input/output code page to UTF-8
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    Pseudocode pseudocode;

    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        help();
        return 0;
    }

    // Parse optional arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug-tokens") {
            pseudocode.debugTokens = true;
        } else if (arg == "--debug-parse") {
            pseudocode.debugParse = true;
        } else {
            // If file ends in .scsa then treat as script
            if (arg.size() < 5 || arg.substr(arg.size() - 5) != ".scsa") {
                help();
                return 1;
            } else {
                return pseudocode.runFile(arg);
            }
        }
    }

    if (argc == 1) {
        return pseudocode.runRepl();
    }
}
