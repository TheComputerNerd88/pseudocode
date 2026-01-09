#pragma once

#include "ast.hpp"
#include "errors.hpp"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <variant>

// Forward declarations
class Environment;
class CallableObject;
class ClassObject;
class InstanceObject;

/**
 * Value - Runtime value representation
 * 
 * Represents any value that can exist during program execution.
 * Uses std::variant to support multiple types with type safety.
 */
struct Value {
    using ValueType = std::variant<
        std::monostate,              // null/void
        double,                      // numbers (int and float both as double)
        std::string,                 // strings
        bool,                        // booleans
        std::vector<Value>,          // arrays
        std::shared_ptr<CallableObject>,  // functions
        std::shared_ptr<InstanceObject>   // class instances
    >;
    
    ValueType data;
    
    // Constructors for each type
    Value() : data(std::monostate{}) {}
    Value(double d) : data(d) {}
    Value(int i) : data(static_cast<double>(i)) {}
    Value(const std::string& s) : data(s) {}
    Value(bool b) : data(b) {}
    Value(const std::vector<Value>& arr) : data(arr) {}
    Value(std::shared_ptr<CallableObject> callable) : data(callable) {}
    Value(std::shared_ptr<InstanceObject> instance) : data(instance) {}
    
    // Type checking helpers
    bool isNull() const { return std::holds_alternative<std::monostate>(data); }
    bool isNumber() const { return std::holds_alternative<double>(data); }
    bool isString() const { return std::holds_alternative<std::string>(data); }
    bool isBool() const { return std::holds_alternative<bool>(data); }
    bool isArray() const { return std::holds_alternative<std::vector<Value>>(data); }
    bool isCallable() const { return std::holds_alternative<std::shared_ptr<CallableObject>>(data); }
    bool isInstance() const { return std::holds_alternative<std::shared_ptr<InstanceObject>>(data); }
    
    // Conversion to string for printing
    std::string toString() const;
    
    // Truthiness for conditionals
    bool isTruthy() const;
};

/**
 * Environment - Variable scope management
 * 
 * Maintains variable bindings in a scope chain.
 * Each environment has an optional parent for lexical scoping.
 */
class Environment {
public:
    Environment() : parent(nullptr) {}
    Environment(std::shared_ptr<Environment> p) : parent(p) {}
    
    void define(const std::string& name, const Value& value);
    Value get(const std::string& name);
    void assign(const std::string& name, const Value& value);
    bool exists(const std::string& name);
    
private:
    std::map<std::string, Value> values;
    std::shared_ptr<Environment> parent;
};

/**
 * CallableObject - Base class for callable entities
 * 
 * Functions and class constructors both implement this interface.
 */
class CallableObject {
public:
    virtual ~CallableObject() = default;
    virtual Value call(class Interpreter& interp, const std::vector<Value>& args) = 0;
    virtual size_t arity() = 0;
    virtual std::string toString() = 0;
};

/**
 * FunctionObject - User-defined function
 * 
 * Captures function declaration and closure environment.
 */
class FunctionObject : public CallableObject {
public:
    FunctionObject(
        const FunctionStmt* decl,
        std::shared_ptr<Environment> closure
    ) : declaration(decl), closure(closure) {}
    
    Value call(Interpreter& interp, const std::vector<Value>& args) override;
    size_t arity() override { return declaration->params.size(); }
    std::string toString() override { return "<fn " + declaration->name.lexeme + ">"; }
    
    const FunctionStmt* declaration;
    std::shared_ptr<Environment> closure;
};

/**
 * ClassObject - Runtime class representation
 * 
 * Stores class metadata and methods.
 */
class ClassObject : public CallableObject {
public:
    ClassObject(
        const std::string& name,
        std::shared_ptr<ClassObject> super,
        std::map<std::string, std::shared_ptr<FunctionObject>> methods
    ) : name(name), superclass(super), methods(methods) {}
    
    Value call(Interpreter& interp, const std::vector<Value>& args) override;
    size_t arity() override;
    std::string toString() override { return "<class " + name + ">"; }
    
    std::shared_ptr<FunctionObject> findMethod(const std::string& name);
    
    std::string name;
    std::shared_ptr<ClassObject> superclass;
    std::map<std::string, std::shared_ptr<FunctionObject>> methods;
};

/**
 * InstanceObject - Runtime instance of a class
 * 
 * Stores instance fields and references its class.
 */
class InstanceObject {
public:
    InstanceObject(std::shared_ptr<ClassObject> klass) : klass(klass) {}
    
    Value get(const std::string& name);
    void set(const std::string& name, const Value& value);
    std::string toString() { return "<" + klass->name + " instance>"; }
    
    std::shared_ptr<ClassObject> klass;
    std::map<std::string, Value> fields;
};

/**
 * ReturnException - Control flow for return statements
 * 
 * Used to unwind the call stack when a return is executed.
 */
class ReturnException : public std::exception {
public:
    ReturnException(const Value& v) : value(v) {}
    Value value;
};

/**
 * Interpreter - Tree-walking interpreter
 * 
 * Executes the AST by recursively evaluating nodes and maintaining
 * runtime state (variables, call stack, etc.).
 */
class Interpreter {
public:
    Interpreter(ErrorReporter& reporter);
    
    /**
     * Execute a program (list of statements)
     */
    void interpret(const std::vector<StmtPtr>& statements);
    
    /**
     * Execute statements in a specific environment
     */
    void executeBlock(const std::vector<StmtPtr>& statements, std::shared_ptr<Environment> env);
    
    // Get current environment (for function closures)
    std::shared_ptr<Environment> getCurrentEnvironment() { return environment; }
    
private:
    ErrorReporter& reporter;
    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> environment;
    
    // Statement execution
    void execute(const Stmt* stmt);
    void executeExpressionStmt(const ExpressionStmt* stmt);
    void executePrintStmt(const PrintStmt* stmt);
    void executeReturnStmt(const ReturnStmt* stmt);
    void executeIfStmt(const IfStmt* stmt);
    void executeWhileStmt(const WhileStmt* stmt);
    void executeFunctionStmt(const FunctionStmt* stmt);
    void executeClassStmt(const ClassStmt* stmt);
    
    // Expression evaluation
    Value evaluate(const Expr* expr);
    Value evaluateLiteral(const LiteralExpr* expr);
    Value evaluateVariable(const VariableExpr* expr);
    Value evaluateAssign(const AssignExpr* expr);
    Value evaluateBinary(const BinaryExpr* expr);
    Value evaluateCall(const CallExpr* expr);
    Value evaluateGet(const GetExpr* expr);
    Value evaluateArrayAccess(const ArrayAccessExpr* expr);
    Value evaluateArrayLit(const ArrayLitExpr* expr);
    Value evaluateNew(const NewExpr* expr);
    
    // Helper functions
    void runtimeError(const std::string& message);
    bool isEqual(const Value& a, const Value& b);
};