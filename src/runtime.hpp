// runtime.hpp
#pragma once

#include "ast.hpp"
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <variant>
#include <vector>

// Forward declarations
struct Callable;
struct Instance;
class Interpreter;

// --- Value Type Definition ---

// A variant to hold any supported runtime value
using Value = std::variant<std::monostate,                                    // null
                           double,                                            // number
                           bool,                                              // boolean
                           std::string,                                       // string
                           std::shared_ptr<std::vector<struct RuntimeValue>>, // Array (shared for
                                                                              // ref semantics)
                           std::shared_ptr<Callable>,                         // Function/Class
                           std::shared_ptr<Instance>                          // Object Instance
                           >;

// Wrapper struct to allow recursive definition in variant (for Arrays)
struct RuntimeValue {
    Value value;

    // Helpers to make code cleaner
    template <typename T> bool is() const {
        return std::holds_alternative<T>(value);
    }
    template <typename T> const T &as() const {
        return std::get<T>(value);
    }
    template <typename T> T &as() {
        return std::get<T>(value);
    }
};

// --- Exceptions ---

// Thrown for runtime errors (e.g., divide by zero)
class RuntimeError : public std::runtime_error {
public:
    const Token &token;
    RuntimeError(const Token &token, const std::string &message)
        : std::runtime_error(message), token(token) {
    }
};

// Thrown to unwind stack on return statement
class ReturnException : public std::exception {
public:
    RuntimeValue value;
    ReturnException(RuntimeValue val) : value(val) {
    }
};

// --- Environment (Scope) ---

class Environment : public std::enable_shared_from_this<Environment> {
    std::map<std::string, RuntimeValue> values;
    std::shared_ptr<Environment> enclosing;

public:
    Environment() : enclosing(nullptr) {
    }
    Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {
    }

    void define(const std::string &name, RuntimeValue value) {
        values[name] = value;
    }

    RuntimeValue get(const Token &name) {
        if (values.count(name.lexeme)) {
            return values.at(name.lexeme);
        }
        if (enclosing)
            return enclosing->get(name);

        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }

    void assign(const Token &name, RuntimeValue value) {
        if (values.count(name.lexeme)) {
            values[name.lexeme] = value;
            return;
        }
        if (enclosing) {
            enclosing->assign(name, value);
            return;
        }

        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }

    // Helper for "ancestor" generic usage if needed later
    std::shared_ptr<Environment> ancestor(int distance);
};

// --- Interfaces for Callables and Instances ---

struct Callable {
    virtual ~Callable() = default;
    virtual int arity() = 0;
    virtual RuntimeValue call(Interpreter &interpreter, std::vector<RuntimeValue> arguments) = 0;
    virtual std::string toString()                                                           = 0;
};

struct Instance {
    std::shared_ptr<Callable> klass; // Reference to class (which is a callable)
    std::map<std::string, RuntimeValue> fields;

    Instance(std::shared_ptr<Callable> k) : klass(k) {
    }

    RuntimeValue get(const Token &name) {
        if (fields.count(name.lexeme)) {
            return fields.at(name.lexeme);
        }
        // In a real implementation, we would look up methods in the 'klass' here
        // For brevity, basic fields only:
        throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
    }

    void set(const Token &name, RuntimeValue value) {
        fields[name.lexeme] = value;
    }
};

// Helper to stringify values
inline std::string stringify(const RuntimeValue &v) {
    if (v.is<std::monostate>())
        return "nil";
    if (v.is<double>()) {
        std::string text = std::to_string(v.as<double>());
        // Trim trailing zeros for integer-like doubles
        text.erase(text.find_last_not_of('0') + 1, std::string::npos);
        if (text.back() == '.')
            text.pop_back();
        return text;
    }
    if (v.is<bool>())
        return v.as<bool>() ? "true" : "false";
    if (v.is<std::string>())
        return v.as<std::string>();
    if (v.is<std::shared_ptr<std::vector<RuntimeValue>>>()) {
        auto arr           = v.as<std::shared_ptr<std::vector<RuntimeValue>>>();
        std::string result = "[";
        for (size_t i = 0; i < arr->size(); ++i) {
            result += stringify((*arr)[i]);
            if (i < arr->size() - 1)
                result += ", ";
        }
        result += "]";
        return result;
    }
    if (v.is<std::shared_ptr<Callable>>())
        return v.as<std::shared_ptr<Callable>>()->toString();
    if (v.is<std::shared_ptr<Instance>>())
        return "Instance";
    return "unknown";
}
