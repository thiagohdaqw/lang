#ifndef __INTERPRETER_H_INCLUDED__
#define __INTERPRETER_H_INCLUDED__

#include "parser.h"
#include "utils/arena.h"

typedef struct {
    const char *key;
    ExprNode *value;
} IVarMap;

typedef struct i_scope {
    struct i_scope *parent;

    IVarMap *vars;
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
    Arena *temp_allocator;
} Interpreter;

Interpreter interpreter_create(Arena *a, Arena *temp_a);
void interpreter_append(Interpreter *i, ExprNode *node);
int interpreter_eval(Interpreter *interpreter);

#endif // __INTERPRETER_H_INCLUDED__

#ifndef __INTERPRETER_H_IMP__
#define __INTERPRETER_H_IMP__

#include "math.h"
#include "utils/stb_ds.h"

void _set_variable(IScope *scope, char *key, ExprNode *right) {
    ExprNode *existing = shget(scope->vars, key);
    if (existing) {
        *existing = *right;
    } else {
        shput(scope->vars, key, right);
    }
}

void _assign_variable(IScope *scope, char *key, ExprNode *right) {
    IScope *current = scope;
    while (1) {
        if (shget(current->vars, key)) {
            break;
        }
        current = current->parent;

        if (!current) {
            current = scope;
            break;
        }
    }
    _set_variable(current, key, right);
}

Interpreter interpreter_create(Arena *a, Arena *temp_a) {
    Interpreter i = {0};
    i.allocator = a;
    i.temp_allocator = temp_a;

    _assign_variable(&i.main, "verdadeiro", node_number(i.allocator, 1));
    _assign_variable(&i.main, "falso", node_number(i.allocator, 0));
    _assign_variable(&i.main, "NULO", node_number(i.allocator, 0));
    _assign_variable(&i.main, "PI", node_number(i.allocator, M_PI));

    return i;
}

IScope *scope_create(Arena *a, IScope *parent) {
    IScope *s = (IScope *)arena_alloc(a, sizeof(*s));
    s->parent = parent;
    return s;
}

void scope_destroy(IScope *scope) {
    shfree(scope->vars);
    da_destroy(scope);
}

void interpreter_append(Interpreter *i, ExprNode *node) { da_append(&i->main, node); }

double eval_number(ExprType op, double a, double b) {
    switch (op) {
    case P_PLUS:
        return a + b;
    case P_MULT:
        return a * b;
    case P_DIV:
        if (b == 0) {
            fprintf(stderr, "Division by zero\n");
            exit(1);
        }
        return a / b;
    case P_POW:
        return pow(a, b);
    default:
        assert(0 && "Unexpected op type");
    }
}

ExprNode *_eval(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a);

ExprNode *_eval_identifier(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a);

ExprNode *_eval_assign(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a);

ExprNode *_get_var(IScope *scope, char *key) {
    while (scope) {
        ExprNode *value = shget(scope->vars, key);
        if (value) {
            return value;
        }
        scope = scope->parent;
    }
    return NULL;
}

ExprNode *_eval_block(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a) {
    ExprNode *ret = NULL;

    if (expr->count == 0) return ret;

    ArenaNode saved_arena = arena_save(a);
    IScope *new_scope = scope_create(a, scope);

    for (size_t i = 0; i < expr->count; i++) {
        ret = _eval(interpreter, new_scope, expr->items[i], a);
        if (ret && ret->returned) {
            break;
        }
        if (ret && ret->type == P_RETURN) {
            ret = ret->first;
            ret->returned = true;
            break;
        }
    }

    if (ret) {
        ret = arena_copy(interpreter->temp_allocator, ret, sizeof(*ret));
    }

    scope_destroy(new_scope);
    arena_rewind(a, saved_arena);

    if (ret) {
        ret = arena_copy(a, ret, sizeof(*ret));
    }

    return ret;
}

ExprNode *_eval_funcall(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a) {
    if (strcmp(expr->string_value, "escreva") == 0) {
        for (size_t i = 0; i < expr->args.count; i++) {
            ExprNode *value = _eval(interpreter, scope, expr->args.items[i], a);
            switch (value->type) {
            case P_STRING:
                printf("%s", value->string_value);
                break;
            case P_NUMBER:
                printf("%lf", value->number_value);
                break;
            default:
                fprintf(stderr, "Print not implemented for type: %d\n", value->type);
                exit(1);
            }
        }
        return NULL;
    }

    ExprNode *func = _get_var(scope, expr->string_value);
    if (!func) {
        fprintf(stderr, "Function '%s' not found\n", expr->string_value);
        exit(1);
    }

    ArenaNode arena_saved = arena_save(a);

    IScope *new_scope = scope_create(a, scope);

    assert(func->args.count == expr->args.count);
    for (size_t i = 0; i < expr->args.count; i++) {
        ExprNode *value = _eval(interpreter, new_scope, expr->args.items[i], a);
        _set_variable(new_scope, func->args.items[i]->string_value, value);
    }

    bool returned;
    ExprNode *ret = _eval_block(interpreter, new_scope, func->first, a);
    if (ret) {
        ret->returned = false;
        ret = arena_copy(interpreter->temp_allocator, ret, sizeof(*ret));
    }

    scope_destroy(new_scope);
    arena_rewind(a, arena_saved);

    if (ret) {
        ret = arena_copy(a, ret, sizeof(*ret));
    }
    return ret;
}

ExprNode *_eval_if(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a) {
    ExprNode *cond = _eval(interpreter, scope, expr->first, a);
    assert(cond && cond->type == P_NUMBER);

    if (cond->number_value) {
        return _eval_block(interpreter, scope, expr->second, a);
    }
    if (expr->third) {
        return _eval_block(interpreter, scope, expr->third, a);
    }
    return NULL;
}

ExprNode *_eval_while(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a) {
    ExprNode *ret = NULL;
    while (1) {
        ExprNode *cond = _eval(interpreter, scope, expr->first, a);
        assert(cond && cond->type == P_NUMBER);

        if (!cond->number_value) break;

        ret = _eval_block(interpreter, scope, expr->second, a);
        if (ret && ret->returned) break;
    }
    return ret;
}

ExprNode *_eval_return(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a) {
    expr->first = _eval(interpreter, scope, expr->first, a);
    return expr;
}

ExprNode *_eval(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a) {
    switch (expr->type) {
    case P_NUMBER:
    case P_STRING:
        return expr;
    case P_IDENTIFIER:
        return _eval_identifier(interpreter, scope, expr, a);
    case P_ASSIGN:
        return _eval_assign(interpreter, scope, expr, a);
    case P_FUN_CALL:
        return _eval_funcall(interpreter, scope, expr, a);
    case P_IF:
        return _eval_if(interpreter, scope, expr, a);
    case P_WHILE:
        return _eval_while(interpreter, scope, expr, a);
    case P_RETURN:
        return _eval_return(interpreter, scope, expr, a);
    case P_PLUS:
    case P_DIV:
    case P_MULT:
    case P_POW: {
        ExprNode *left = _eval(interpreter, scope, expr->first, a);
        assert(left && "Unexpected left null");
        assert(left->type == P_NUMBER && "Left is not a number");
        ExprNode *right = _eval(interpreter, scope, expr->second, a);
        assert(right && "Unexpected right null");
        assert(right->type == P_NUMBER && "Right is not a number");
        double result = eval_number(expr->type, left->number_value, right->number_value);
        return node_number(a, result);
    }
    case P_MINUS: {
        ExprNode *result = _eval(interpreter, scope, expr->first, a);
        assert(result && "Unexpected minus result null");
        assert(result->type == P_NUMBER && "Minus result is not a number");
        return node_number(a, (-1) * result->number_value);
    }
    case P_FUNC: {
        if (shget(scope->vars, expr->string_value)) {
            fprintf(stderr, "Function '%s' already defined", expr->string_value);
            exit(1);
        }
        _assign_variable(scope, expr->string_value, expr);
        return expr;
    }
    default:
        fprintf(stderr, "Expr type not implemented: %d\n", expr->type);
        assert(0 && "Expr type not implemented");
    }
}

ExprNode *_eval_identifier(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a) {
    ExprNode *id = _get_var(scope, expr->string_value);
    if (!id) {
        fprintf(stderr, "Identifier '%s' not found\n", expr->string_value);
        exit(1);
    }
    ExprNode *value = _eval(interpreter, scope, id, a);
    return id;
}

ExprNode *_eval_assign(Interpreter *interpreter, IScope *scope, ExprNode *expr, Arena *a) {
    ExprNode *right = _eval(interpreter, scope, expr->second, a);
    _assign_variable(scope, expr->first->string_value, right);
    return right;
}

int interpreter_eval(Interpreter *interpreter) {
    ExprNode *result = NULL;
    for (size_t i = 0; i < interpreter->main.count; i++) {
        ArenaNode temp_saved = arena_save(interpreter->temp_allocator);

        result = _eval(interpreter, &interpreter->main, interpreter->main.items[i], interpreter->allocator);

        arena_rewind(interpreter->temp_allocator, temp_saved);
    }

    fflush(stdout);

    if (result && result->type == P_RETURN) {
        return result->first->number_value;
    }
    return 0;
}

#endif // __INTERPRETER_H_IMP__