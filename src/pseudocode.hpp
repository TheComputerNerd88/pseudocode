#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

#include "lexer.hpp"
#include "errors.hpp"
#include "interpreter.hpp"

class Pseudocode {
public:
    // Run parsing on a file path. Returns process exit code.
    static int runFile(const std::string& path);

    // REPL
    static int runRepl();

private:
    static std::string readFile(const std::string& path);

    static void printTokenTable(const std::vector<Token>& tokens);
};
