#include "errors.hpp"

/**
 * Berate the user because they had an error
 */
void printAtarMessage() {
    std::cout << C_RED << "[SCSA] Your ATAR is cooked, -99999 marks." <<
        std::endl << "[SCSA] Congratulations, you are the first student" <<
        " to ever get a negative study score! 😭" << std::endl <<
        "[SCSA] Say goodbye to your future. L + ratio 😂 😂"
        << C_RESET << std::endl;
}

/**
 * ErrorReporter Constructor
 * Stores a reference to the current interpreter stage for error reporting
 * @param stageRef Reference to the current InterpreterStage
 */
ErrorReporter::ErrorReporter(InterpreterStage& stageRef) : stage(stageRef) {}

/**
 * Map error types to human-readable labels
 * @param type The error type to convert
 * @return String representation of the error type
 */
std::string ErrorReporter::getErrorLabel(ErrorType type) {
    switch (type) {
        case ErrorType::Syntax:     return "Syntax Error";
        case ErrorType::Type:       return "Type Error";
        case ErrorType::Runtime:    return "Runtime Error";
        default:                    return "Unknown Error";
    }
}

/**
 * Map interpreter stages to human-readable labels
 * @return String representation of the current stage
 */
std::string ErrorReporter::getStageLabel() {
    switch (stage) {
        case InterpreterStage::Lexing:   return "Lexing";
        case InterpreterStage::Parsing:  return "Parsing";
        case InterpreterStage::Runtime:  return "Runtime";
        default:                         return "Unknown";
    }
}

/**
 * Format and display a complete error message with context
 * Shows the error stage, location, line content, and error pointer
 * @param type The type of error (Syntax, Type, etc.)
 * @param line The line number where the error occurred
 * @param column The column position where the error occurred
 * @param message The error message to display
 * @param lineSource The actual source code line containing the error
 */
void ErrorReporter::report(ErrorType type, size_t line, size_t column,
        const std::string& message, const std::string& lineSource) {
    // Print which stage the interpreter is in
    std::string stageLabel = getStageLabel();
    std::cerr << C_RED << "[An error occurred during stage: '" << stageLabel
        << "']" << std::endl;

    // Print error message with location information
    std::string label = getErrorLabel(type);
    std::cerr << "[Line " << C_BLUE << line << ":" << column + 1 <<
        C_RED << "] " << label << ": " << message << C_RESET << std::endl;
    
    // Print the problematic line of code
    std::cerr << C_RED << ">>  " << C_RESET << lineSource << std::endl;
    
    // Generate a pointer string to indicate the exact error position
    std::cerr << "    ";

    // Account for tab characters when positioning the error pointer
    for (size_t i = 0; i < column; i++) {
        if (i < lineSource.length() && lineSource[i] == '\t') {
            std::cerr << '\t';
        } else {
            std::cerr << ' ';
        }
    }

    std::cerr << C_RED << "^" << C_RESET << std::endl;

    // Print a message from the SCSA since the user has clearly failed
    // printAtarMessage();

    // Throw exception to halt execution
    throw std::runtime_error("");
}
