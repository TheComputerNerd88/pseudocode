#include "errors.hpp"

void printAtarMessage() {
    std::cout << C_RED << "[SCSA] Your ATAR is cooked, -99999 marks." <<
        std::endl << "[SCSA] Congratulations, you are the first student" <<
        " to ever get a negative study score! 😭" << std::endl <<
        "[SCSA] Say goodbye to your future. L + ratio 😂 😂"
        << C_RESET << std::endl;
}

ErrorReporter::ErrorReporter(InterpreterStage& stageRef) : stage(stageRef) {}

std::string ErrorReporter::getErrorLabel(ErrorType type) {
    switch (type) {
        case ErrorType::Syntax: return "Syntax Error";
        case ErrorType::Type:   return "Type Error";
        default:                return "Error";
    }
}

std::string ErrorReporter::getStageLabel() {
    switch (stage) {
        case InterpreterStage::Lexing:   return "Lexing";
        case InterpreterStage::Parsing:  return "Parsing";
        case InterpreterStage::Runtime:  return "Runtime";
        default:                         return "huh";
    }
}

void ErrorReporter::report(ErrorType type, size_t line, size_t column,
        const std::string& message, const std::string& lineSource) {
    // Print which stage the interpreter is in
    std::string stageLabel = getStageLabel();
    std::cerr << C_RED << "[Error occurred during stage: '" << stageLabel
        << "']" << std::endl;

    // Print error message
    std::string label = getErrorLabel(type);
    std::cerr << "[Line " << C_BLUE << line << ":" << column + 1 <<
        C_RED << "] " << label << ": " << message << C_RESET << std::endl;
    
    
    // Print the Line of Code
    std::cerr << C_RED << ">>  " << C_RESET << lineSource << std::endl;
    
    // Generate the Pointer string
    std::cerr << "    ";

    for (size_t i = 0; i < column; i++) {
        if (i < lineSource.length() && lineSource[i] == '\t') {
            std::cerr << '\t';
        } else {
            std::cerr << ' ';
        }
    }

    std::cerr << C_RED << "^" << C_RESET << std::endl;

    printAtarMessage();

    throw std::runtime_error("");
}
