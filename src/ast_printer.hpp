#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "parser.hpp" // or wherever your structs are defined

/**
 * ASTPrinter - Outputs an Abstract Syntax Tree in human-readable tree format
 * 
 * Recursively walks the AST and prints each node with proper indentation,
 * making it easy to visualize the structure and relationships between nodes.
 * Useful for debugging parser output and understanding program structure.
 */
class ASTPrinter {
public:
    /**
     * Entry point for printing
     * Outputs the entire AST starting from a list of top-level statements
     * @param statements Vector of statement nodes to print
     */
    void print(const std::vector<StmtPtr>& statements);

private:
    /**
     * Recursively print a statement node with indentation
     * @param stmt Pointer to statement node to print
     * @param indent Current indentation prefix
     */
    void printStmt(const Stmt* stmt, std::string indent);
    
    /**
     * Recursively print an expression node with indentation
     * @param expr Pointer to expression node to print
     * @param indent Current indentation prefix
     */
    void printExpr(const Expr* expr, std::string indent);

    /**
     * Helper to increase indentation level for nested nodes
     * @param indent Current indentation string
     * @return New indentation string with additional indent level
     */
    std::string increaseIndent(std::string indent);
};
