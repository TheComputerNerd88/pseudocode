#include "errors.hpp"

/**
 * Berate the user because they had an error
 */
void printAtarMessage() {
    std::cout << C_RED << "[SCSA] Your ATAR is cooked, -99999 marks." << std::endl
              << "[SCSA] Congratulations, you are the first student"
              << " to ever get a negative study score! 😭" << std::endl
              << "[SCSA] Say goodbye to your future. L + ratio 😂 😂" << C_RESET << std::endl;
}

/**
 * ErrorReporter Constructor
 * Stores a reference to the current interpreter stage, filename, and source code for error reporting
 * @param stageRef Reference to the current InterpreterStage
 * @param file The source filename being processed
 * @param source The full source code for context generation
 */
ErrorReporter::ErrorReporter(InterpreterStage &stageRef, const std::string &file, const std::string &source) 
    : stage(stageRef), filename(file), sourceCode(source) {
}

/**
 * Map error types to human-readable labels
 * @param type The error type to convert
 * @return String representation of the error type
 */
std::string ErrorReporter::getErrorLabel(ErrorType type) {
    switch (type) {
    case ErrorType::Syntax:
        return "Syntax Error";
    case ErrorType::Type:
        return "Type Error";
    case ErrorType::Runtime:
        return "Runtime Error";
    default:
        return "Unknown Error";
    }
}

/**
 * Map interpreter stages to human-readable labels
 * @return String representation of the current stage
 */
std::string ErrorReporter::getStageLabel() {
    switch (stage) {
    case InterpreterStage::Lexing:
        return "Lexing";
    case InterpreterStage::Parsing:
        return "Parsing";
    case InterpreterStage::Runtime:
        return "Runtime";
    default:
        return "Unknown";
    }
}

/**
 * Extract a specific line from the source code
 * @param lineNum The 1-based line number to extract
 * @return The source code line, or empty string if out of range
 */
std::string ErrorReporter::getSourceLine(size_t lineNum) {
    if (sourceCode.empty() || lineNum < 1) {
        return "";
    }

    size_t currentLine = 1;
    size_t lineStart = 0;

    for (size_t i = 0; i < sourceCode.length(); i++) {
        if (i > 0 && sourceCode[i - 1] == '\n') {
            currentLine++;
            if (currentLine == lineNum) {
                lineStart = i;
                break;
            }
        } else if (i == 0 && lineNum == 1) {
            lineStart = 0;
            break;
        }
    }

    // If we never found the line, return empty
    if (currentLine != lineNum && lineNum != 1) {
        return "";
    }

    // Find the end of this line
    size_t lineEnd = lineStart;
    while (lineEnd < sourceCode.length() && sourceCode[lineEnd] != '\n') {
        lineEnd++;
    }

    return sourceCode.substr(lineStart, lineEnd - lineStart);
}

/**
 * Format and display a complete error message with context
 * Shows the error stage, location, line content, and error pointer
 * @param type The type of error (Syntax, Type, etc.)
 * @param line The line number where the error occurred
 * @param column The column position where the error occurred
 * @param message The error message to display
 * @param lineSource The actual source code line containing the error
 * @param length The length of the erroneous token (for underlining)
 */
/**
 * Format and display a complete error message with context
 * Shows the error stage, location, line content, surrounding lines, and error pointer
 */
void ErrorReporter::report(ErrorType type, size_t line, size_t column, const std::string &message,
                           size_t length) {
    // Print which stage the interpreter is in
    std::string stageLabel = getStageLabel();
    std::cerr << C_RED << "[An error has occurred during the stage: '" << stageLabel << "']"
              << std::endl;

    // Get the error line
    std::string errorLine = getSourceLine(line);
    
    // Create coordinate string
    std::string coords = std::to_string(line) + ":" + std::to_string(column + 1);
    std::string separator = " │ "; 
    size_t gutterWidth = coords.length() + 3;

    // Print filename (Aligned)
    if (!filename.empty()) {
        std::string indent(gutterWidth - 2, ' ');
        std::cerr << C_BLUE << indent << "┌─ " << filename << C_RESET << std::endl;
    }

    // Print line before (if it exists)
    if (line > 1) {
        std::string prevLine = getSourceLine(line - 1);
        if (!prevLine.empty()) {
            std::string prevCoords = std::to_string(line - 1);
            std::cerr << C_BLUE << prevCoords;
            // Pad to match the width of the error line number
            for (size_t i = prevCoords.length(); i < coords.length(); i++) {
                std::cerr << " ";
            }
            std::cerr << separator << C_RESET << C_GRAY << prevLine << C_RESET << std::endl;
        }
    }

    // Print the error line with highlighting
    std::cerr << C_BLUE << coords << separator << C_RESET;

    // Print the line up to the error
    std::cerr << errorLine.substr(0, column);

    // Print the erroneous token in red
    std::cerr << C_RED << errorLine.substr(column, length) << C_RESET;

    // Print the rest of the line
    if (column + length < errorLine.length()) {
        std::cerr << errorLine.substr(column + length);
    }
    std::cerr << std::endl;

    // Print line after (if it exists)
    if (!getSourceLine(line + 1).empty()) {
        std::string nextLine = getSourceLine(line + 1);
        std::string nextCoords = std::to_string(line + 1);
        std::cerr << C_GRAY << nextCoords;
        // Pad to match the width of the error line number
        for (size_t i = nextCoords.length(); i < coords.length(); i++) {
            std::cerr << " ";
        }
        std::cerr << separator << nextLine << C_RESET << std::endl;
    }

    // Generate the caret alignment on error line
    // Print spaces equal to the width of the gutter
    for (size_t i = 0; i < gutterWidth; i++) {
        std::cerr << ' ';
    }

    // Account for tab characters inside the source code itself
    for (size_t i = 0; i < column; i++) {
        if (i < errorLine.length() && errorLine[i] == '\t') {
            std::cerr << '\t';
        } else {
            std::cerr << ' ';
        }
    }

    // Draw the underline carets
    std::cerr << C_RED;
    for (size_t i = 0; i < length; i++) {
        std::cerr << '^';
    }

    // Print error label and message next to the carets
    std::string label = getErrorLabel(type);
    std::cerr << " " << label << ": " << C_RESET << message << std::endl;

    // Lovely message from our overlords SCSA
    printAtarMessage();

    throw std::runtime_error("");
}
