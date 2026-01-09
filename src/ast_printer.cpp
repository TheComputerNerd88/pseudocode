#include "ast_printer.hpp"

#include <iostream>

/**
 * Helper to construct the indentation string for tree visualization
 * Each level adds "  | " to visually show the tree structure
 */
std::string ASTPrinter::increaseIndent(std::string indent) {
    return indent + "  | ";
}

/**
 * Entry point for AST printing
 * Prints "AST Root" as the tree header, then recursively prints each top-level statement
 */
void ASTPrinter::print(const std::vector<StmtPtr>& statements) {
    std::cout << "AST Root" << std::endl;
    for (const auto& stmt : statements) {
        printStmt(stmt.get(), "");
    }
}

// ==========================================
// Statement Printer
// 
// Handles printing of different statement types by dynamic casting
// and delegating to appropriate output logic
// ==========================================

/**
 * Recursively print a statement with indentation
 * Dispatches to appropriate printing logic based on statement type
 */
void ASTPrinter::printStmt(const Stmt* stmt, std::string indent) {
    std::string childIndent = increaseIndent(indent);

    // Class Declaration
    if (auto* s = dynamic_cast<const ClassStmt*>(stmt)) {
        std::cout << indent << "[Class] " << s->name.lexeme;
        if (s->superclass.type != TOK_EOF) {
            std::cout << " < " << s->superclass.lexeme;
        }
        std::cout << std::endl;
        for (const auto& method : s->methods) {
            printStmt(method.get(), childIndent);
        }
        return;
    }

    // Function Declaration
    if (auto* s = dynamic_cast<const FunctionStmt*>(stmt)) {
        std::cout << indent << "[Function] " << s->name.lexeme << "(";
        for (size_t i = 0; i < s->params.size(); ++i) {
            std::cout << s->params[i].lexeme << (i < s->params.size() - 1 ? ", " : "");
        }
        std::cout << ")" << std::endl;
        for (const auto& bodyStmt : s->body) {
            printStmt(bodyStmt.get(), childIndent);
        }
        return;
    }

    // If Statement
    if (auto* s = dynamic_cast<const IfStmt*>(stmt)) {
        std::cout << indent << "[If]" << std::endl;
        
        std::cout << childIndent << "Condition:" << std::endl;
        printExpr(s->condition.get(), increaseIndent(childIndent));

        std::cout << childIndent << "Then:" << std::endl;
        for (const auto& st : s->thenBranch) printStmt(st.get(), increaseIndent(childIndent));

        if (!s->elseBranch.empty()) {
            std::cout << childIndent << "Else:" << std::endl;
            for (const auto& st : s->elseBranch) printStmt(st.get(), increaseIndent(childIndent));
        }
        return;
    }

    // While Statement
    if (auto* s = dynamic_cast<const WhileStmt*>(stmt)) {
        std::cout << indent << "[While]" << std::endl;
        std::cout << childIndent << "Condition:" << std::endl;
        printExpr(s->condition.get(), increaseIndent(childIndent));
        std::cout << childIndent << "Body:" << std::endl;
        for (const auto& st : s->body) printStmt(st.get(), increaseIndent(childIndent));
        return;
    }

    // Return Statement
    if (auto* s = dynamic_cast<const ReturnStmt*>(stmt)) {
        std::cout << indent << "[Return]" << std::endl;
        if (s->value) printExpr(s->value.get(), childIndent);
        return;
    }

    // Print Statement
    if (auto* s = dynamic_cast<const PrintStmt*>(stmt)) {
        std::cout << indent << "[Print]" << std::endl;
        printExpr(s->expression.get(), childIndent);
        return;
    }

    // Expression Statement
    if (auto* s = dynamic_cast<const ExpressionStmt*>(stmt)) {
        std::cout << indent << "[ExprStmt]" << std::endl;
        printExpr(s->expression.get(), childIndent);
        return;
    }
    
    std::cout << indent << "[Unknown Stmt]" << std::endl;
}

// ==========================================
// Expression Printer
// 
// Handles printing of different expression types by dynamic casting
// and recursively printing sub-expressions with proper indentation
// ==========================================

/**
 * Recursively print an expression with indentation
 * Dispatches to appropriate printing logic based on expression type
 */
void ASTPrinter::printExpr(const Expr* expr, std::string indent) {
    if (!expr) return;
    std::string childIndent = increaseIndent(indent);

    // Binary Operation
    if (auto* e = dynamic_cast<const BinaryExpr*>(expr)) {
        std::cout << indent << "Binary (" << e->op.lexeme << ")" << std::endl;
        printExpr(e->left.get(), childIndent);
        printExpr(e->right.get(), childIndent);
        return;
    }

    // Assignment
    if (auto* e = dynamic_cast<const AssignExpr*>(expr)) {
        std::cout << indent << "Assign (=)" << std::endl;
        std::cout << childIndent << "Target:" << std::endl;
        printExpr(e->target.get(), increaseIndent(childIndent));
        std::cout << childIndent << "Value:" << std::endl;
        printExpr(e->value.get(), increaseIndent(childIndent));
        return;
    }

    // Literal
    if (auto* e = dynamic_cast<const LiteralExpr*>(expr)) {
        std::cout << indent << "Literal: " << e->token.lexeme << std::endl;
        return;
    }

    // Variable
    if (auto* e = dynamic_cast<const VariableExpr*>(expr)) {
        std::cout << indent << "Var: " << e->name.lexeme << std::endl;
        return;
    }

    // Function Call
    if (auto* e = dynamic_cast<const CallExpr*>(expr)) {
        std::cout << indent << "Call" << std::endl;
        std::cout << childIndent << "Callee:" << std::endl;
        printExpr(e->callee.get(), increaseIndent(childIndent));
        std::cout << childIndent << "Args:" << std::endl;
        for (const auto& arg : e->args) {
            printExpr(arg.get(), increaseIndent(childIndent));
        }
        return;
    }

    // Property Access
    if (auto* e = dynamic_cast<const GetExpr*>(expr)) {
        std::cout << indent << "Get Property: ." << e->name.lexeme << std::endl;
        printExpr(e->object.get(), childIndent);
        return;
    }

    // Array Subscript
    if (auto* e = dynamic_cast<const ArrayAccessExpr*>(expr)) {
        std::cout << indent << "Array Index []" << std::endl;
        std::cout << childIndent << "Array:" << std::endl;
        printExpr(e->array.get(), increaseIndent(childIndent));
        std::cout << childIndent << "Index:" << std::endl;
        printExpr(e->index.get(), increaseIndent(childIndent));
        return;
    }

    // Array Literal
    if (auto* e = dynamic_cast<const ArrayLitExpr*>(expr)) {
        std::cout << indent << "Array Literal []" << std::endl;
        for(const auto& elem : e->elements) {
             printExpr(elem.get(), childIndent);
        }
        return;
    }
    
    // Object Instantiation
    if (auto* e = dynamic_cast<const NewExpr*>(expr)) {
        std::cout << indent << "New " << e->className.lexeme << std::endl;
        for(const auto& arg : e->args) {
             printExpr(arg.get(), childIndent);
        }
        return;
    }

    std::cout << indent << "[Unknown Expr]" << std::endl;
}
