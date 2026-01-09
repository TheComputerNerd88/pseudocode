#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "interpreter.hpp"

// Every type of error we'll need
enum class ErrorType {
    Syntax,  // Syntax errors (invalid token sequences)
    Type,    // Type errors (type mismatches)
    Runtime, // Runtime errors (errors during execuction)
};

// ANSI color codes for terminal output
#define C_RED "\033[31m"
#define C_RESET "\033[0m"
#define C_BLUE "\e[34m"

/**
 * Print an error message from our benevolent overlord SCSA
 */
void printAtarMessage();

/**
 * ErrorReporter class handles all error reporting and formatting
 * Provides context about where the error occurred in the source code
 */
class ErrorReporter {
private:
    // Reference to current interpreter stage
    InterpreterStage& stage;

    /**
     * Get human-readable error type label
     */
    std::string getErrorLabel(ErrorType type);
    
    /**
     * Get human-readable interpreter stage label
     */
    std::string getStageLabel();

public:
    /**
     * Construct error reporter with reference to current stage
     * @param stageRef Reference to the interpreter stage
     */
    ErrorReporter(InterpreterStage& stageRef);

    /**
     * Report an error with full context
     * @param type The error type (Syntax, Type, etc.)
     * @param line The line number where error occurred
     * @param column The column position where error occurred
     * @param message The error message to display
     * @param lineSource The source code line containing the error
     */
    void report(ErrorType type, size_t line, size_t column,
            const std::string& message, const std::string& lineSource);
};
