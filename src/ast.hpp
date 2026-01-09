#pragma once

#include <vector>
#include <string>
#include <memory>

#include "lexer.hpp"

// Forward declarations
struct Expr;
struct Stmt;

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

// --- Expressions ---

struct Expr {
    virtual ~Expr() = default;
};

struct LiteralExpr : Expr {
    Token token; // Holds INT, FLOAT, STRING, TRUE, FALSE
    LiteralExpr(Token t) : token(t) {}
};

struct VariableExpr : Expr {
    Token name;
    VariableExpr(Token n) : name(n) {}
};

struct AssignExpr : Expr {
    ExprPtr target; // Supports array/dot assignment: arr[0] = 1
    ExprPtr value;
    AssignExpr(ExprPtr t, ExprPtr v) : target(std::move(t)), value(std::move(v)) {}
};

struct BinaryExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, Token o, ExprPtr r) 
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

struct CallExpr : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> args;
    CallExpr(ExprPtr c, std::vector<ExprPtr> a) 
        : callee(std::move(c)), args(std::move(a)) {}
};

struct GetExpr : Expr {
    ExprPtr object;
    Token name;
    GetExpr(ExprPtr o, Token n) : object(std::move(o)), name(n) {}
};

struct ArrayAccessExpr : Expr {
    ExprPtr array;
    ExprPtr index;
    ArrayAccessExpr(ExprPtr a, ExprPtr i) : array(std::move(a)), index(std::move(i)) {}
};

struct ArrayLitExpr : Expr {
    std::vector<ExprPtr> elements;
    ArrayLitExpr(std::vector<ExprPtr> e) : elements(std::move(e)) {}
};

struct NewExpr : Expr {
    Token className;
    std::vector<ExprPtr> args;
    NewExpr(Token c, std::vector<ExprPtr> a) : className(c), args(std::move(a)) {}
};

// --- Statements ---

struct Stmt {
    virtual ~Stmt() = default;
};

struct ExpressionStmt : Stmt {
    ExprPtr expression;
    ExpressionStmt(ExprPtr e) : expression(std::move(e)) {}
};

struct PrintStmt : Stmt {
    ExprPtr expression;
    PrintStmt(ExprPtr e) : expression(std::move(e)) {}
};

struct ReturnStmt : Stmt {
    ExprPtr value;
    ReturnStmt(ExprPtr v) : value(std::move(v)) {}
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
    BlockStmt(std::vector<StmtPtr> s) : statements(std::move(s)) {}
};

struct IfStmt : Stmt {
    ExprPtr condition;
    std::vector<StmtPtr> thenBranch;
    std::vector<StmtPtr> elseBranch; // Empty if no else
    IfStmt(ExprPtr c, std::vector<StmtPtr> t, std::vector<StmtPtr> e)
        : condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
};

struct WhileStmt : Stmt {
    ExprPtr condition;
    std::vector<StmtPtr> body;
    WhileStmt(ExprPtr c, std::vector<StmtPtr> b) : condition(std::move(c)), body(std::move(b)) {}
};

struct FunctionStmt : Stmt {
    Token name;
    std::vector<Token> params;
    std::vector<StmtPtr> body;
    FunctionStmt(Token n, std::vector<Token> p, std::vector<StmtPtr> b)
        : name(n), params(p), body(std::move(b)) {}
};

struct ClassStmt : Stmt {
    Token name;
    Token superclass; // Optional, Type=EOF if none
    std::vector<StmtPtr> methods; // FunctionStmts
    // Note: Attributes could be stored here too, simplifying to just Stmts for now
    ClassStmt(Token n, Token s, std::vector<StmtPtr> m)
        : name(n), superclass(s), methods(std::move(m)) {}
};
