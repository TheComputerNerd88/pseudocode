#include "interpreter.hpp"

// --- Helper Functions ---

void Interpreter::execute(Stmt *stmt) {
    if (stmt)
        stmt->accept(*this);
}

RuntimeValue Interpreter::evaluate(Expr *expr) {
    expr->accept(*this);
    return result;
}

bool Interpreter::isTruthy(const RuntimeValue &object) {
    if (object.is<std::monostate>())
        return false;
    if (object.is<bool>())
        return object.as<bool>();
    return true;
}

bool Interpreter::isEqual(const RuntimeValue &a, const RuntimeValue &b) {
    // Rely on variant equality operators or implement specific logic
    if (a.value.index() != b.value.index())
        return false;
    if (a.is<std::monostate>())
        return true;
    if (a.is<double>())
        return a.as<double>() == b.as<double>();
    if (a.is<bool>())
        return a.as<bool>() == b.as<bool>();
    if (a.is<std::string>())
        return a.as<std::string>() == b.as<std::string>();
    return false; // For objects/arrays, this implies reference equality checks usually
}

void Interpreter::checkNumberOperand(const Token &operatorToken, const RuntimeValue &operand) {
    if (operand.is<double>())
        return;
    throw RuntimeError(operatorToken, "Operand must be a number.");
}

void Interpreter::checkNumberOperands(const Token &operatorToken, const RuntimeValue &left,
                                      const RuntimeValue &right) {
    if (left.is<double>() && right.is<double>())
        return;
    throw RuntimeError(operatorToken, "Operands must be numbers.");
}

void Interpreter::executeBlock(const std::vector<StmtPtr> &statements,
                               std::shared_ptr<Environment> env) {
    std::shared_ptr<Environment> previous = this->environment;
    try {
        this->environment = env;
        for (const auto &stmt : statements) {
            execute(stmt.get());
        }
    } catch (...) {
        // Restore environment even if exception (Return/RuntimeError) occurs
        this->environment = previous;
        throw;
    }
    this->environment = previous;
}

// --- ExprVisitor Implementation ---

void Interpreter::visitLiteralExpr(LiteralExpr *expr) {
    switch (expr->token.type) {
    case TOK_FALSE:
        result = {false};
        break;
    case TOK_TRUE:
        result = {true};
        break;
    case TOK_STRING:
        result = {expr->token.lexeme};
        break; // Lexeme contains quotes, usually stripped in lexer
    case TOK_INTEGER:
    case TOK_FLOAT:
        result = {std::stod(expr->token.lexeme)};
        break;
    default:
        result = {std::monostate{}};
        break;
    }
}

void Interpreter::visitVariableExpr(VariableExpr *expr) {
    result = environment->get(expr->name);
}

void Interpreter::visitAssignExpr(AssignExpr *expr) {
    RuntimeValue value = evaluate(expr->value.get());

    // Check if target is a simple variable
    if (auto varExpr = dynamic_cast<VariableExpr *>(expr->target.get())) {
        try {
            environment->assign(varExpr->name, value);
        } catch (const RuntimeError &) {
            // Variable doesn't exist, define it
            environment->define(varExpr->name.lexeme, value);
        }
    }
    // Check if target is a property set (object.prop = val)
    else if (auto getExpr = dynamic_cast<GetExpr *>(expr->target.get())) {
        RuntimeValue object = evaluate(getExpr->object.get());
        if (object.is<std::shared_ptr<Instance>>()) {
            object.as<std::shared_ptr<Instance>>()->set(getExpr->name, value);
        } else {
            throw RuntimeError(getExpr->name, "Only instances have fields.");
        }
    }
    // Check if target is array index (arr[i] = val)
    else if (auto arrExpr = dynamic_cast<ArrayAccessExpr *>(expr->target.get())) {
        RuntimeValue arrVal = evaluate(arrExpr->array.get());
        RuntimeValue idxVal = evaluate(arrExpr->index.get());

        if (!arrVal.is<std::shared_ptr<std::vector<RuntimeValue>>>()) {
            // We need a token for error reporting, simplified here
            throw std::runtime_error("Cannot assign to non-array subscript.");
        }
        if (!idxVal.is<double>()) {
            throw std::runtime_error("Array index must be a number.");
        }

        auto vec  = arrVal.as<std::shared_ptr<std::vector<RuntimeValue>>>();
        int index = (int) idxVal.as<double>();

        if (index < 0 || index >= (int) vec->size()) {
            throw std::runtime_error("Array index out of bounds.");
        }
        (*vec)[index] = value;
    } else {
        throw std::runtime_error("Invalid assignment target.");
    }

    result = value;
}

void Interpreter::visitBinaryExpr(BinaryExpr *expr) {
    RuntimeValue left  = evaluate(expr->left.get());
    RuntimeValue right = evaluate(expr->right.get());

    switch (expr->op.type) {
    case TOK_GREATER_THAN:
        checkNumberOperands(expr->op, left, right);
        result = {left.as<double>() > right.as<double>()};
        break;
    case TOK_LESS_THAN:
        checkNumberOperands(expr->op, left, right);
        result = {left.as<double>() < right.as<double>()};
        break;
    case TOK_MINUS:
        checkNumberOperands(expr->op, left, right);
        result = {left.as<double>() - right.as<double>()};
        break;
    case TOK_DIVIDE:
        checkNumberOperands(expr->op, left, right);
        if (right.as<double>() == 0)
            throw RuntimeError(expr->op, "Division by zero.");
        result = {left.as<double>() / right.as<double>()};
        break;
    case TOK_MULTIPLY:
        checkNumberOperands(expr->op, left, right);
        result = {left.as<double>() * right.as<double>()};
        break;
    case TOK_PLUS:
        if (left.is<double>() && right.is<double>()) {
            result = {left.as<double>() + right.as<double>()};
        } else if (left.is<std::string>() && right.is<std::string>()) {
            result = {left.as<std::string>() + right.as<std::string>()};
        } else {
            throw RuntimeError(expr->op, "Operands must be two numbers or two strings.");
        }
        break;
    case TOK_EQUAL:
        result = {isEqual(left, right)};
        break;
    default:
        break;
    }
}

void Interpreter::visitCallExpr(CallExpr *expr) {
    RuntimeValue callee = evaluate(expr->callee.get());

    std::vector<RuntimeValue> args;
    for (const auto &arg : expr->args) {
        args.push_back(evaluate(arg.get()));
    }

    if (!callee.is<std::shared_ptr<Callable>>()) {
        // Can't locate token easily from CallExpr without storing it,
        // assuming callee expression has location info or throw generic
        throw std::runtime_error("Can only call functions and classes.");
    }

    auto function = callee.as<std::shared_ptr<Callable>>();
    if ((int) args.size() != function->arity()) {
        throw std::runtime_error("Expected " + std::to_string(function->arity()) +
                                 " arguments but got " + std::to_string(args.size()) + ".");
    }

    result = function->call(*this, args);
}

void Interpreter::visitGetExpr(GetExpr *expr) {
    RuntimeValue object = evaluate(expr->object.get());
    if (object.is<std::shared_ptr<Instance>>()) {
        result = object.as<std::shared_ptr<Instance>>()->get(expr->name);
        return;
    }
    throw RuntimeError(expr->name, "Only instances have properties.");
}

void Interpreter::visitArrayAccessExpr(ArrayAccessExpr *expr) {
    RuntimeValue arr = evaluate(expr->array.get());
    RuntimeValue idx = evaluate(expr->index.get());

    if (!arr.is<std::shared_ptr<std::vector<RuntimeValue>>>()) {
        throw std::runtime_error("Operand not an array.");
    }
    if (!idx.is<double>()) {
        throw std::runtime_error("Index must be a number.");
    }

    auto vec  = arr.as<std::shared_ptr<std::vector<RuntimeValue>>>();
    int index = (int) idx.as<double>();

    if (index < 0 || index >= (int) vec->size()) {
        throw std::runtime_error("Index out of bounds.");
    }

    result = (*vec)[index];
}

void Interpreter::visitArrayLitExpr(ArrayLitExpr *expr) {
    auto vec = std::make_shared<std::vector<RuntimeValue>>();
    for (const auto &el : expr->elements) {
        vec->push_back(evaluate(el.get()));
    }
    result = {vec};
}

void Interpreter::visitNewExpr(NewExpr *expr) {
    // Look up class
    RuntimeValue klassVal = environment->get(expr->className);
    if (!klassVal.is<std::shared_ptr<Callable>>()) {
        throw RuntimeError(expr->className, "Can only instantiate classes.");
    }

    // Evaluate args
    std::vector<RuntimeValue> args;
    for (const auto &arg : expr->args) {
        args.push_back(evaluate(arg.get()));
    }

    // Call the class (constructor logic)
    result = klassVal.as<std::shared_ptr<Callable>>()->call(*this, args);
}

// --- StmtVisitor Implementation ---

void Interpreter::visitExpressionStmt(ExpressionStmt *stmt) {
    evaluate(stmt->expression.get());
}

void Interpreter::visitPrintStmt(PrintStmt *stmt) {
    RuntimeValue val = evaluate(stmt->expression.get());
    std::cout << stringify(val) << std::endl;
}

void Interpreter::visitReturnStmt(ReturnStmt *stmt) {
    RuntimeValue value = {std::monostate{}};
    if (stmt->value) {
        value = evaluate(stmt->value.get());
    }
    throw ReturnException(value);
}

void Interpreter::visitBlockStmt(BlockStmt *stmt) {
    executeBlock(stmt->statements, std::make_shared<Environment>(environment));
}

void Interpreter::visitIfStmt(IfStmt *stmt) {
    if (isTruthy(evaluate(stmt->condition.get()))) {
        for (const auto &s : stmt->thenBranch)
            execute(s.get());
    } else {
        for (const auto &s : stmt->elseBranch)
            execute(s.get());
    }
}

void Interpreter::visitWhileStmt(WhileStmt *stmt) {
    while (isTruthy(evaluate(stmt->condition.get()))) {
        for (const auto &s : stmt->body)
            execute(s.get());
    }
}

void Interpreter::visitForInStmt(ForInStmt *stmt) {
    RuntimeValue iterable = evaluate(stmt->iterable.get());

    if (!iterable.is<std::shared_ptr<std::vector<RuntimeValue>>>()) {
        throw RuntimeError(stmt->variable, "For-in loop requires an array.");
    }

    auto vec = iterable.as<std::shared_ptr<std::vector<RuntimeValue>>>();

    // Iterate through C++ vector
    for (const auto &val : *vec) {
        // Create a new scope for the loop variable
        auto loopEnv = std::make_shared<Environment>(environment);
        loopEnv->define(stmt->variable.lexeme, val);

        executeBlock(stmt->body, loopEnv);
    }
}

// --- Function & Class Definitions ---

// User Defined Function Implementation
class LoxFunction : public Callable {
    FunctionStmt *declaration;
    std::shared_ptr<Environment> closure;

public:
    LoxFunction(FunctionStmt *decl, std::shared_ptr<Environment> closure)
        : declaration(decl), closure(closure) {
    }

    int arity() override {
        return declaration->params.size() || 0;
    }

    RuntimeValue call(Interpreter &interpreter, std::vector<RuntimeValue> arguments) override {
        auto environment = std::make_shared<Environment>(closure);
        for (size_t i = 0; i < declaration->params.size(); ++i) {
            environment->define(declaration->params[i].lexeme, arguments[i]);
        }

        try {
            interpreter.executeBlock(declaration->body, environment);
        } catch (const ReturnException &returnValue) {
            return returnValue.value;
        }
        return {std::monostate{}};
    }

    std::string toString() override {
        return "<fn " + declaration->name.lexeme + ">";
    }
};

void Interpreter::visitFunctionStmt(FunctionStmt *stmt) {
    auto function = std::make_shared<LoxFunction>(stmt, environment);
    environment->define(stmt->name.lexeme, {function});
}

// Class Definition Implementation
class LoxClass : public Callable, public std::enable_shared_from_this<LoxClass> {
    std::string name;
    // Methods would be stored here in a full implementation
public:
    LoxClass(std::string name) : name(name) {
    }

    int arity() override {
        return 0;
    } // Assuming no-arg constructor for simplicity

    RuntimeValue call(Interpreter & /*interpreter*/,
                      std::vector<RuntimeValue> /*arguments*/) override {
        auto instance = std::make_shared<Instance>(shared_from_this());
        return {instance};
    }

    std::string toString() override {
        return "<class " + name + ">";
    }
};

void Interpreter::visitClassStmt(ClassStmt *stmt) {
    environment->define(stmt->name.lexeme,
                        {std::monostate{}}); // Define nil first to allow recursion
    auto klass = std::make_shared<LoxClass>(stmt->name.lexeme);
    environment->assign(stmt->name, {klass});
}
