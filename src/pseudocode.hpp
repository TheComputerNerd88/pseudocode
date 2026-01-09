#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

#include "lexer.hpp"
#include "parser.hpp"
#include "errors.hpp"
/**
 * Main Pseudocode interpreter class
 * Provides both file execution and interactive REPL modes
 * Currently supports lexical analysis and token output
 */
class Pseudocode {
public:
    /**
     * Execute pseudocode from a file
     * @param path Path to the pseudocode file to run
     * @return 0 on success, 1 on error
     */
    static int runFile(const std::string& path);

    /**
     * Run an interactive REPL (Read-Eval-Print-Loop)
     * Allows users to type pseudocode lines and see tokenization
     * @return Always returns 0
     */
    static int runRepl();
private:
    /**
     * Read entire file contents into a string
     * @param path Path to the file to read
     * @return File contents as string
     */
    static std::string readFile(const std::string& path);

    /**
     * Print tokens in a formatted table
     * @param tokens Vector of tokens to display
     */
    static void printTokenTable(const std::vector<Token>& tokens);
};
