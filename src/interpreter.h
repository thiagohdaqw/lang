#ifndef __INTERPRETER_H_INCLUDED__
#define __INTERPRETER_H_INCLUDED__

#include "parser.h"
#include "utils/arena.h"

typedef struct {
    const char *key;
    ExprNode *value;
} IVar;

typedef struct {
    IVar *vars;
    ExprNode **items;
    int count;
    int capacity;
} IScope;

typedef struct {
    IScope main;
    IScope items;
    int count;
    int capacity;

    Arena *allocator;
} Interpreter;

Interpreter interpreter_create(Arena *a);
void interpreter_append(Interpreter *i, ExprNode *node);
void interpreter_eval(Interpreter *interpreter);

#endif // __INTERPRETER_H_INCLUDED__

#ifndef __INTERPRETER_H_IMP__
#define __INTERPRETER_H_IMP__

Interpreter interpreter_create(Arena *a) {
    Interpreter i = {0};
    i.allocator = a;
    return i;
}

void interpreter_append(Interpreter *i, ExprNode *node) { da_append(&i->main, node); }

double eval_number(ExprType op, double a, double b) {
    switch (op) {
    case P_PLUS:
        return a + b;
    case P_MULT:
        return a * b;
    case P_DIV:
        return a / b;
    default:
        assert(0 && "Unexpected op type");
    }
}

ExprNode *_eval(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a);

ExprNode *_eval_funcall(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a) {
    for (size_t i = 0; i < expr->count; i++) {
        expr->args.items[i] = _eval(interpreter, scope, expr->args.items[i], a);
    }

    if (strcmp(expr->string_value, "print") == 0) {
        assert(expr->args.count == 1);
        switch (expr->args.items[0]->type) {
        case P_STRING:
            printf("%s", expr->args.items[0]->string_value);
            break;
        case P_NUMBER:
            printf("%lf\n", expr->args.items[0]->number_value);
            break;
        }
        return NULL;
    }
    fprintf(stderr, "Function call '%s' not implemented: %d\n", expr->string_value, strlen(expr->string_value));
    assert(0 && "Func not implemented");
}

ExprNode *_eval(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a) {
    switch (expr->type) {
    case P_IDENTIFIER: {
        ExprNode *id = hmget(scope->vars, expr->string_value);
        assert(id && "Identifier not found");
        ExprNode *value = _eval(interpreter, scope, id, a);
        return value;
    }
    case P_NUMBER:
    case P_STRING:
        return expr;
    case P_ASSIGN: {
        ExprNode *right = _eval(interpreter, scope, expr->right, a);

        hmput(scope->vars, expr->left->string_value, right);
        return right;
    }
    case P_FUN_CALL:
        return _eval_funcall(interpreter, scope, expr, a);
    case P_PLUS:
    case P_DIV:
    case P_MULT: {
        ExprNode *left = _eval(interpreter, scope, expr->left, a);
        assert(left && "Unexpected left null");
        assert(left->type == P_NUMBER && "Left is not a number");
        ExprNode *right = _eval(interpreter, scope, expr->right, a);
        assert(right && "Unexpected right null");
        assert(right->type == P_NUMBER && "Right is not a number");
        double result = eval_number(expr->type, left->number_value, right->number_value);
        return node_number(a, result);
    }
    case P_MINUS: {
        ExprNode *result = _eval(interpreter, scope, expr->right, a);
        assert(result && "Unexpected minus result null");
        assert(result->type == P_NUMBER && "Minus result is not a number");
        return node_number(a, (-1) * result->number_value);
    }
    case P_FUNC:
        return NULL;
    default:
        assert(0 && "Type not implemented");
    }
}

void interpreter_eval(Interpreter *interpreter) {
    ExprNode *result = NULL;
    for (size_t i = 0; i < interpreter->main.count; i++) {
        result = _eval(interpreter, &interpreter->main, interpreter->main.items[i], interpreter->allocator);
    }

    parser_print_expression(result);
}

#endif // __INTERPRETER_H_IMP__