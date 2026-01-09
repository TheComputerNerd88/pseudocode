#include "tree-walker.hpp"
#include <iostream>
#include <cmath>

// ============================================================
// Value Implementation
// ============================================================

std::string Value::toString() const {
    if (isNull()) return "null";
    if (isNumber()) {
        double num = std::get<double>(data);
        // Check if it's an integer value
        if (num == std::floor(num)) {
            return std::to_string(static_cast<int>(num));
        }
        return std::to_string(num);
    }
    if (isString()) return std::get<std::string>(data);
    if (isBool()) return std::get<bool>(data) ? "true" : "false";
    if (isArray()) {
        const auto& arr = std::get<std::vector<Value>>(data);
        std::string result = "[";
        for (size_t i = 0; i < arr.size(); ++i) {
            result += arr[i].toString();
            if (i < arr.size() - 1) result += ", ";
        }
        result += "]";
        return result;
    }
    if (isCallable()) {
        return std::get<std::shared_ptr<CallableObject>>(data)->toString();
    }
    if (isInstance()) {
        return std::get<std::shared_ptr<InstanceObject>>(data)->toString();
    }
    return "unknown";
}

bool Value::isTruthy() const {
    if (isNull()) return false;
    if (isBool()) return std::get<bool>(data);
    if (isNumber()) return std::get<double>(data) != 0.0;
    if (isString()) return !std::get<std::string>(data).empty();
    // Objects, arrays, functions are truthy
    return true;
}

// ============================================================
// Environment Implementation
// ============================================================

void Environment::define(const std::string& name, const Value& value) {
    values[name] = value;
}

Value Environment::get(const std::string& name) {
    auto it = values.find(name);
    if (it != values.end()) {
        return it->second;
    }
    if (parent) {
        return parent->get(name);
    }
    throw std::runtime_error("Undefined variable '" + name + "'.");
}

void Environment::assign(const std::string& name, const Value& value) {
    auto it = values.find(name);
    if (it != values.end()) {
        it->second = value;
        return;
    }
    if (parent) {
        parent->assign(name, value);
        return;
    }
    throw std::runtime_error("Undefined variable '" + name + "'.");
}

bool Environment::exists(const std::string& name) {
    if (values.find(name) != values.end()) return true;
    if (parent) return parent->exists(name);
    return false;
}

// ============================================================
// FunctionObject Implementation
// ============================================================

Value FunctionObject::call(Interpreter& interp, const std::vector<Value>& args) {
    // Create new environment for function execution
    auto functionEnv = std::make_shared<Environment>(closure);
    
    // Bind parameters to arguments
    for (size_t i = 0; i < declaration->params.size(); ++i) {
        functionEnv->define(declaration->params[i].lexeme, args[i]);
    }
    
    // Execute function body
    try {
        interp.executeBlock(declaration->body, functionEnv);
    } catch (const ReturnException& ret) {
        return ret.value;
    }
    
    return Value(); // Implicit null return
}

// ============================================================
// ClassObject Implementation
// ============================================================

Value ClassObject::call(Interpreter& interp, const std::vector<Value>& args) {
    // Create new instance
    auto instance = std::make_shared<InstanceObject>(
        std::shared_ptr<ClassObject>(this, [](ClassObject*){})); // Non-owning shared_ptr
    
    // Call constructor if exists
    auto constructor = findMethod("constructor");
    if (constructor) {
        // Bind 'this' to constructor
        auto constructorEnv = std::make_shared<Environment>(constructor->closure);
        constructorEnv->define("this", Value(instance));
        
        // Execute constructor with bound 'this'
        for (size_t i = 0; i < args.size(); ++i) {
            if (i < constructor->declaration->params.size()) {
                constructorEnv->define(constructor->declaration->params[i].lexeme, args[i]);
            }
        }
        
        try {
            interp.executeBlock(constructor->declaration->body, constructorEnv);
        } catch (const ReturnException&) {
            // Constructors shouldn't return values, but ignore if they do
        }
    }
    
    return Value(instance);
}

size_t ClassObject::arity() {
    auto constructor = findMethod("constructor");
    if (constructor) return constructor->arity();
    return 0;
}

std::shared_ptr<FunctionObject> ClassObject::findMethod(const std::string& name) {
    auto it = methods.find(name);
    if (it != methods.end()) {
        return it->second;
    }
    if (superclass) {
        return superclass->findMethod(name);
    }
    return nullptr;
}

// ============================================================
// InstanceObject Implementation
// ============================================================

Value InstanceObject::get(const std::string& name) {
    // Check fields first
    auto it = fields.find(name);
    if (it != fields.end()) {
        return it->second;
    }
    
    // Check methods
    auto method = klass->findMethod(name);
    if (method) {
        return Value(method);
    }
    
    throw std::runtime_error("Undefined property '" + name + "'.");
}

void InstanceObject::set(const std::string& name, const Value& value) {
    fields[name] = value;
}

// ============================================================
// Interpreter Implementation
// ============================================================

Interpreter::Interpreter(ErrorReporter& reporter) 
    : reporter(reporter) {
    globals = std::make_shared<Environment>();
    environment = globals;
}

void Interpreter::interpret(const std::vector<StmtPtr>& statements) {
    try {
        for (const auto& stmt : statements) {
            execute(stmt.get());
        }
    } catch (const std::exception& e) {
        runtimeError(e.what());
    }
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& statements, 
                                std::shared_ptr<Environment> env) {
    auto previous = environment;
    try {
        environment = env;
        for (const auto& stmt : statements) {
            execute(stmt.get());
        }
        environment = previous;
    } catch (...) {
        environment = previous;
        throw;
    }
}

// ============================================================
// Statement Execution
// ============================================================

void Interpreter::execute(const Stmt* stmt) {
    if (auto* s = dynamic_cast<const ExpressionStmt*>(stmt)) {
        executeExpressionStmt(s);
    } else if (auto* s = dynamic_cast<const PrintStmt*>(stmt)) {
        executePrintStmt(s);
    } else if (auto* s = dynamic_cast<const ReturnStmt*>(stmt)) {
        executeReturnStmt(s);
    } else if (auto* s = dynamic_cast<const IfStmt*>(stmt)) {
        executeIfStmt(s);
    } else if (auto* s = dynamic_cast<const WhileStmt*>(stmt)) {
        executeWhileStmt(s);
    } else if (auto* s = dynamic_cast<const FunctionStmt*>(stmt)) {
        executeFunctionStmt(s);
    } else if (auto* s = dynamic_cast<const ClassStmt*>(stmt)) {
        executeClassStmt(s);
    }
}

void Interpreter::executeExpressionStmt(const ExpressionStmt* stmt) {
    evaluate(stmt->expression.get());
}

void Interpreter::executePrintStmt(const PrintStmt* stmt) {
    Value value = evaluate(stmt->expression.get());
    std::cout << value.toString() << std::endl;
}

void Interpreter::executeReturnStmt(const ReturnStmt* stmt) {
    Value value;
    if (stmt->value) {
        value = evaluate(stmt->value.get());
    }
    throw ReturnException(value);
}

void Interpreter::executeIfStmt(const IfStmt* stmt) {
    Value condition = evaluate(stmt->condition.get());
    
    if (condition.isTruthy()) {
        for (const auto& s : stmt->thenBranch) {
            execute(s.get());
        }
    } else if (!stmt->elseBranch.empty()) {
        for (const auto& s : stmt->elseBranch) {
            execute(s.get());
        }
    }
}

void Interpreter::executeWhileStmt(const WhileStmt* stmt) {
    while (evaluate(stmt->condition.get()).isTruthy()) {
        for (const auto& s : stmt->body) {
            execute(s.get());
        }
    }
}

void Interpreter::executeFunctionStmt(const FunctionStmt* stmt) {
    auto function = std::make_shared<FunctionObject>(stmt, environment);
    environment->define(stmt->name.lexeme, Value(function));
}

void Interpreter::executeClassStmt(const ClassStmt* stmt) {
    // Handle superclass
    std::shared_ptr<ClassObject> superclass = nullptr;
    if (stmt->superclass.type != TOK_EOF) {
        Value superValue = environment->get(stmt->superclass.lexeme);
        if (!superValue.isCallable()) {
            throw std::runtime_error("Superclass must be a class.");
        }
        auto callable = std::get<std::shared_ptr<CallableObject>>(superValue.data);
        superclass = std::dynamic_pointer_cast<ClassObject>(callable);
        if (!superclass) {
            throw std::runtime_error("Superclass must be a class.");
        }
    }
    
    // Create environment for class methods
    auto classEnv = environment;
    if (superclass) {
        classEnv = std::make_shared<Environment>(environment);
        classEnv->define("super", Value(std::static_pointer_cast<CallableObject>(superclass)));
    }
    
    // Collect methods
    std::map<std::string, std::shared_ptr<FunctionObject>> methods;
    for (const auto& method : stmt->methods) {
        if (auto* funcStmt = dynamic_cast<const FunctionStmt*>(method.get())) {
            auto funcObj = std::make_shared<FunctionObject>(funcStmt, classEnv);
            methods[funcStmt->name.lexeme] = funcObj;
        }
    }
    
    auto klass = std::make_shared<ClassObject>(stmt->name.lexeme, superclass, methods);
    environment->define(stmt->name.lexeme, Value(std::static_pointer_cast<CallableObject>(klass)));
}

// ============================================================
// Expression Evaluation
// ============================================================

Value Interpreter::evaluate(const Expr* expr) {
    if (auto* e = dynamic_cast<const LiteralExpr*>(expr)) {
        return evaluateLiteral(e);
    } else if (auto* e = dynamic_cast<const VariableExpr*>(expr)) {
        return evaluateVariable(e);
    } else if (auto* e = dynamic_cast<const AssignExpr*>(expr)) {
        return evaluateAssign(e);
    } else if (auto* e = dynamic_cast<const BinaryExpr*>(expr)) {
        return evaluateBinary(e);
    } else if (auto* e = dynamic_cast<const CallExpr*>(expr)) {
        return evaluateCall(e);
    } else if (auto* e = dynamic_cast<const GetExpr*>(expr)) {
        return evaluateGet(e);
    } else if (auto* e = dynamic_cast<const ArrayAccessExpr*>(expr)) {
        return evaluateArrayAccess(e);
    } else if (auto* e = dynamic_cast<const ArrayLitExpr*>(expr)) {
        return evaluateArrayLit(e);
    } else if (auto* e = dynamic_cast<const NewExpr*>(expr)) {
        return evaluateNew(e);
    }
    return Value();
}

Value Interpreter::evaluateLiteral(const LiteralExpr* expr) {
    switch (expr->token.type) {
        case TOK_INTEGER:
            return Value(std::stoi(expr->token.lexeme));
        case TOK_FLOAT:
            return Value(std::stod(expr->token.lexeme));
        case TOK_STRING:
            return Value(expr->token.lexeme);
        case TOK_TRUE:
            return Value(true);
        case TOK_FALSE:
            return Value(false);
        default:
            return Value();
    }
}

Value Interpreter::evaluateVariable(const VariableExpr* expr) {
    return environment->get(expr->name.lexeme);
}

Value Interpreter::evaluateAssign(const AssignExpr* expr) {
    Value value = evaluate(expr->value.get());
    
    // Handle different assignment targets
    if (auto* varExpr = dynamic_cast<const VariableExpr*>(expr->target.get())) {
        // Simple variable assignment
        if (!environment->exists(varExpr->name.lexeme)) {
            environment->define(varExpr->name.lexeme, value);
        } else {
            environment->assign(varExpr->name.lexeme, value);
        }
    } else if (auto* getExpr = dynamic_cast<const GetExpr*>(expr->target.get())) {
        // Property assignment: obj.prop = value
        Value object = evaluate(getExpr->object.get());
        if (!object.isInstance()) {
            throw std::runtime_error("Only instances have properties.");
        }
        auto instance = std::get<std::shared_ptr<InstanceObject>>(object.data);
        instance->set(getExpr->name.lexeme, value);
    } else if (auto* arrExpr = dynamic_cast<const ArrayAccessExpr*>(expr->target.get())) {
        // Array assignment: arr[idx] = value
        Value arrayVal = evaluate(arrExpr->array.get());
        if (!arrayVal.isArray()) {
            throw std::runtime_error("Can only index arrays.");
        }
        Value indexVal = evaluate(arrExpr->index.get());
        if (!indexVal.isNumber()) {
            throw std::runtime_error("Array index must be a number.");
        }
        
        auto& arr = std::get<std::vector<Value>>(arrayVal.data);
        int idx = static_cast<int>(std::get<double>(indexVal.data));
        if (idx < 0 || idx >= static_cast<int>(arr.size())) {
            throw std::runtime_error("Array index out of bounds.");
        }
        arr[idx] = value;
    }
    
    return value;
}

Value Interpreter::evaluateBinary(const BinaryExpr* expr) {
    Value left = evaluate(expr->left.get());
    Value right = evaluate(expr->right.get());
    
    switch (expr->op.type) {
        case TOK_PLUS:
            if (left.isNumber() && right.isNumber()) {
                return Value(std::get<double>(left.data) + std::get<double>(right.data));
            }
            if (left.isString() && right.isString()) {
                return Value(std::get<std::string>(left.data) + std::get<std::string>(right.data));
            }
            throw std::runtime_error("Operands must be two numbers or two strings.");
            
        case TOK_MINUS:
            if (!left.isNumber() || !right.isNumber()) {
                throw std::runtime_error("Operands must be numbers.");
            }
            return Value(std::get<double>(left.data) - std::get<double>(right.data));
            
        case TOK_MULTIPLY:
            if (!left.isNumber() || !right.isNumber()) {
                throw std::runtime_error("Operands must be numbers.");
            }
            return Value(std::get<double>(left.data) * std::get<double>(right.data));
            
        case TOK_DIVIDE:
            if (!left.isNumber() || !right.isNumber()) {
                throw std::runtime_error("Operands must be numbers.");
            }
            if (std::get<double>(right.data) == 0.0) {
                throw std::runtime_error("Division by zero.");
            }
            return Value(std::get<double>(left.data) / std::get<double>(right.data));
            
        case TOK_EQUAL:
            return Value(isEqual(left, right));
            
        case TOK_GREATER_THAN:
            if (!left.isNumber() || !right.isNumber()) {
                throw std::runtime_error("Operands must be numbers.");
            }
            return Value(std::get<double>(left.data) > std::get<double>(right.data));
            
        case TOK_GT_OR_EQ:
            if (!left.isNumber() || !right.isNumber()) {
                throw std::runtime_error("Operands must be numbers.");
            }
            return Value(std::get<double>(left.data) >= std::get<double>(right.data));
            
        case TOK_LESS_THAN:
            if (!left.isNumber() || !right.isNumber()) {
                throw std::runtime_error("Operands must be numbers.");
            }
            return Value(std::get<double>(left.data) < std::get<double>(right.data));
            
        case TOK_LT_OR_EQ:
            if (!left.isNumber() || !right.isNumber()) {
                throw std::runtime_error("Operands must be numbers.");
            }
            return Value(std::get<double>(left.data) <= std::get<double>(right.data));
            
        case TOK_IN:
            if (!right.isArray()) {
                throw std::runtime_error("'IN' operator requires an array on the right.");
            }
            for (const auto& elem : std::get<std::vector<Value>>(right.data)) {
                if (isEqual(left, elem)) {
                    return Value(true);
                }
            }
            return Value(false);
            
        default:
            throw std::runtime_error("Unknown binary operator.");
    }
}

Value Interpreter::evaluateCall(const CallExpr* expr) {
    Value callee = evaluate(expr->callee.get());
    
    std::vector<Value> args;
    for (const auto& arg : expr->args) {
        args.push_back(evaluate(arg.get()));
    }
    
    if (!callee.isCallable()) {
        throw std::runtime_error("Can only call functions and classes.");
    }
    
    auto callable = std::get<std::shared_ptr<CallableObject>>(callee.data);
    
    if (args.size() != callable->arity()) {
        throw std::runtime_error(
            "Expected " + std::to_string(callable->arity()) + 
            " arguments but got " + std::to_string(args.size()) + "."
        );
    }
    
    return callable->call(*this, args);
}

Value Interpreter::evaluateGet(const GetExpr* expr) {
    Value object = evaluate(expr->object.get());
    
    if (!object.isInstance()) {
        throw std::runtime_error("Only instances have properties.");
    }
    
    auto instance = std::get<std::shared_ptr<InstanceObject>>(object.data);
    return instance->get(expr->name.lexeme);
}

Value Interpreter::evaluateArrayAccess(const ArrayAccessExpr* expr) {
    Value array = evaluate(expr->array.get());
    Value index = evaluate(expr->index.get());
    
    if (!array.isArray()) {
        throw std::runtime_error("Can only index arrays.");
    }
    if (!index.isNumber()) {
        throw std::runtime_error("Array index must be a number.");
    }
    
    const auto& arr = std::get<std::vector<Value>>(array.data);
    int idx = static_cast<int>(std::get<double>(index.data));
    
    if (idx < 0 || idx >= static_cast<int>(arr.size())) {
        throw std::runtime_error("Array index out of bounds.");
    }
    
    return arr[idx];
}

Value Interpreter::evaluateArrayLit(const ArrayLitExpr* expr) {
    std::vector<Value> elements;
    for (const auto& elem : expr->elements) {
        elements.push_back(evaluate(elem.get()));
    }
    return Value(elements);
}

Value Interpreter::evaluateNew(const NewExpr* expr) {
    Value classValue = environment->get(expr->className.lexeme);
    
    if (!classValue.isCallable()) {
        throw std::runtime_error("Can only instantiate classes.");
    }
    
    auto callable = std::get<std::shared_ptr<CallableObject>>(classValue.data);
    auto klass = std::dynamic_pointer_cast<ClassObject>(callable);
    
    if (!klass) {
        throw std::runtime_error("Can only instantiate classes.");
    }
    
    std::vector<Value> args;
    for (const auto& arg : expr->args) {
        args.push_back(evaluate(arg.get()));
    }
    
    return klass->call(*this, args);
}

// ============================================================
// Helper Functions
// ============================================================

void Interpreter::runtimeError(const std::string& message) {
    reporter.report(ErrorType::Runtime, 0, 0, message, "");
}

bool Interpreter::isEqual(const Value& a, const Value& b) {
    if (a.isNull() && b.isNull()) return true;
    if (a.isNull() || b.isNull()) return false;
    
    if (a.isNumber() && b.isNumber()) {
        return std::get<double>(a.data) == std::get<double>(b.data);
    }
    if (a.isString() && b.isString()) {
        return std::get<std::string>(a.data) == std::get<std::string>(b.data);
    }
    if (a.isBool() && b.isBool()) {
        return std::get<bool>(a.data) == std::get<bool>(b.data);
    }
    
    return false;
}