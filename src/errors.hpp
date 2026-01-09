#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "interpreter.hpp"

// Every type of error we'll need
enum class ErrorType {
    Syntax,
    Type,
};

#define C_RED "\033[31m"
#define C_RESET "\033[0m"
#define C_BLUE "\e[34m"

void printAtarMessage();

class ErrorReporter {
private:
    // Reference to current interpreter stage
    InterpreterStage& stage;

    // Label maps
    std::string getErrorLabel(ErrorType type);
    std::string getStageLabel();

public:
    ErrorReporter(InterpreterStage& stageRef);

    void report(ErrorType type, size_t line, size_t column,
            const std::string& message, const std::string& lineSource);
};
