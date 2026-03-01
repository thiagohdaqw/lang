#ifndef __INTERPRETER_H_INCLUDED__
#define __INTERPRETER_H_INCLUDED__

#include "utils/arena.h"
#include "parser.h"

ExprNode *interpreter_eval(ExprNode *expr, Arena *a);

#endif // __INTERPRETER_H_INCLUDED__

#ifndef __INTERPRETER_H_IMP__
#define __INTERPRETER_H_IMP__

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

ExprNode *_eval(ExprNode *expr, Arena *a) {
    switch (expr->type) {
    case P_NUMBER:
        return expr;
    case P_IDENTIFIER:
        return expr;
    case P_ASSIGN: {
        ExprNode *left = interpreter_eval(expr->left, a);
        ExprNode *right = interpreter_eval(expr->right, a);
        return node_op(a, T_ASSIGN, left, right);
    }
    case P_PLUS:
    case P_DIV:
    case P_MULT: {
        ExprNode *left = interpreter_eval(expr->left, a);
        assert(left && "Unexpected left null");
        assert(left->type == P_NUMBER && "Left is not a number");
        ExprNode *right = interpreter_eval(expr->right, a);
        assert(right && "Unexpected right null");
        assert(right->type == P_NUMBER && "Right is not a number");
        double result =
            eval_number(expr->type, left->number_value, right->number_value);
        return node_number(a, result);
    }
    case P_MINUS: {
        ExprNode *result = interpreter_eval(expr->right, a);
        assert(result && "Unexpected minus result null");
        assert(result->type == P_NUMBER && "Minus result is not a number");
        return node_number(a, (-1) * result->number_value);
    }
    }
    return NULL;
}

ExprNode *interpreter_eval(ExprNode *expr, Arena *a) { return _eval(expr, a); }

#endif // __INTERPRETER_H_IMP__