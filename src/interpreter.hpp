#pragma once

#include "ast.hpp"
#include "errors.hpp"
#include "runtime.hpp"
#include <memory>
#include <vector>

class Interpreter : public ExprVisitor, public StmtVisitor {
public:
    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> environment;

    Interpreter() {
        globals     = std::make_shared<Environment>();
        environment = globals;

        // Define native functions (e.g., clock, print) in globals here if needed
    }

    void interpret(const std::vector<StmtPtr> &statements) {
        try {
            for (const auto &stmt : statements) {
                execute(stmt.get());
            }
        } catch (const RuntimeError &error) {
            std::cerr << "[Runtime Error] " << error.what() << "\n[Line " << error.token.line << "]"
                      << std::endl;
        }
    }

    // Public API for executing blocks (used by Callables)
    void executeBlock(const std::vector<StmtPtr> &statements, std::shared_ptr<Environment> env);

    // --- ExprVisitor ---
    void visitLiteralExpr(LiteralExpr *expr) override;
    void visitVariableExpr(VariableExpr *expr) override;
    void visitAssignExpr(AssignExpr *expr) override;
    void visitBinaryExpr(BinaryExpr *expr) override;
    void visitCallExpr(CallExpr *expr) override;
    void visitGetExpr(GetExpr *expr) override;
    void visitArrayAccessExpr(ArrayAccessExpr *expr) override;
    void visitArrayLitExpr(ArrayLitExpr *expr) override;
    void visitNewExpr(NewExpr *expr) override;

    // --- StmtVisitor ---
    void visitExpressionStmt(ExpressionStmt *stmt) override;
    void visitPrintStmt(PrintStmt *stmt) override;
    void visitReturnStmt(ReturnStmt *stmt) override;
    void visitBlockStmt(BlockStmt *stmt) override;
    void visitIfStmt(IfStmt *stmt) override;
    void visitWhileStmt(WhileStmt *stmt) override;
    void visitFunctionStmt(FunctionStmt *stmt) override;
    void visitClassStmt(ClassStmt *stmt) override;
    void visitForInStmt(ForInStmt *stmt) override;

private:
    // Helper to hold the result of expression evaluation
    RuntimeValue result;

    void execute(Stmt *stmt);
    RuntimeValue evaluate(Expr *expr);

    // Truthiness logic (false and nil are false, everything else true)
    bool isTruthy(const RuntimeValue &object);
    bool isEqual(const RuntimeValue &a, const RuntimeValue &b);
    void checkNumberOperand(const Token &operatorToken, const RuntimeValue &operand);
    void checkNumberOperands(const Token &operatorToken, const RuntimeValue &left,
                             const RuntimeValue &right);
};
