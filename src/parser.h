#ifndef __PARSER_H_INCLUDED__
#define __PARSER_H_INCLUDED__

#include "arena.h"
#include "lexer.h"

typedef enum {
    P_NOP,
    P_NUMBER,
    P_CHAR,
    P_STRING,
    P_ASSIGN,
    P_BINOP,
    P_PLUS,
    P_MINUS,
    P_IDENTIFIER,
    P_DIV,
    P_MULT,
} ExprType;

typedef struct op_node_t {
    ExprType type;

    double number_value;
    char char_value;
    char *string_value;
    struct op_node_t *left;
    struct op_node_t *right;
} ExprNode;

ExprNode *parser_expression(Lexer *lexer, Arena *arena);
void parser_print_expression(ExprNode *expr);
ExprNode *parser_eval_expr(ExprNode *expr, Arena *a);

#endif // __PARSER_H_INCLUDED__

#ifndef __PARSER_H_IMP__

ExprNode *node_number(Arena *a, double value) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_NUMBER;
    node->number_value = value;
    return node;
}

ExprNode *node_plus(Arena *a, ExprNode *left, ExprNode *right) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_PLUS;
    node->left = left;
    node->right = right;
    return node;
}

ExprNode *node_mult(Arena *a, ExprNode *left, ExprNode *right) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_MULT;
    node->left = left;
    node->right = right;
    return node;
}

ExprNode *node_div(Arena *a, ExprNode *left, ExprNode *right) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_DIV;
    node->left = left;
    node->right = right;
    return node;
}

ExprNode *node_minus(Arena *a, ExprNode *right) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_MINUS;
    node->left = NULL;
    node->right = right;
    return node;
}

ExprNode *node_op(Arena *a, TokenType op, ExprNode *left, ExprNode *right) {
    switch (op) {
    case T_PLUS:
        return node_plus(a, left, right);
    case T_MINUS:
        return node_plus(a, left, node_minus(a, right));
    case T_MULT:
        return node_mult(a, left, right);
    case T_DIV:
        return node_div(a, left, right);
    default:
        assert(0 && "Unexpected op node type");
    }
}

ExprNode *node_identifier(Arena *a, const char *value) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_IDENTIFIER;
    node->string_value = arena_strdup(a, value);
    return node;
}

ExprNode *node_assign(Arena *a, ExprNode *left, ExprNode *right) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_ASSIGN;
    node->left = left;
    node->right = right;
    return node;
}

int get_infix_power(TokenType type) {
    switch (type) {
    case T_MULT:
    case T_DIV:
        return 60;
    case T_PLUS:
    case T_MINUS:
        return 50;
    case T_ASSIGN:
        return -10;
    default:
        return 0;
    }
}

ExprNode *_parser_expression(Lexer *lexer, Arena *arena, int min_power);

ExprNode *_parse_unop(Lexer *lexer, Arena *arena) {
    if (!lexer_next_token(lexer))
        return NULL;

    switch (lexer->token.type) {
    case T_NEW_LINE:
        return NULL;
    case T_LONG:
        return node_number(arena, (double)lexer->token.long_value);
    case T_DOUBLE:
        return node_number(arena, lexer->token.double_value);
    case T_LITERAL:
        if (lexer->token.literal_value == '-') {
            return node_minus(arena, _parser_expression(lexer, arena, 70));
        }
        break;
    case T_IDENTIFIER:
        return node_identifier(arena, lexer->token.identifier_value);
    default:
        LEXER_ERROR_PRINT(lexer, "Unexpected unop type: %d\n",
                          lexer->token.type);
        exit(1);
    }
}

ExprNode *_parser_expression(Lexer *lexer, Arena *arena, int power) {
    ExprNode *left = _parse_unop(lexer, arena);
    if (!left)
        return left;

    Reader r = lexer_save_reader(lexer);

    while (1) {
        r = lexer_save_reader(lexer);
        if (!lexer_next_token(lexer))
            return left;

        switch (lexer->token.type) {
        case T_PLUS:
        case T_MINUS:
        case T_MULT:
        case T_DIV: {
            char op = lexer->token.type;
            int op_power = get_infix_power(op);

            if (op_power <= power)
                goto rewind;
            ExprNode *right = _parser_expression(lexer, arena, op_power);
            if (!right) {
                LEXER_ERROR_PRINT(lexer, "Unexpected token after %c\n", op);
                exit(1);
            }
            left = node_op(arena, op, left, right);
        } break;
        case T_ASSIGN: {
            if (left->type != P_IDENTIFIER) {
                LEXER_ERROR_PRINT(
                    lexer, "Token left than assign must be a identifier: %d\n",
                    left->type);
                exit(1);
            }
            ExprNode *right =
                _parser_expression(lexer, arena, get_infix_power(T_ASSIGN));
            if (!right) {
                LEXER_ERROR_PRINT(lexer, "Unexpected token to assign\n");
                exit(1);
            }
            return node_assign(arena, left, right);
        } break;
        case T_NEW_LINE:
            goto rewind;
        default:
            LEXER_ERROR_PRINT(lexer, "Unexpected token: %d\n",
                              lexer->token.type);
            assert(0 && "unexpected token");
        }
    }
rewind:
    lexer_rewind_reader(lexer, r);
    return left;
}

ExprNode *parser_expression(Lexer *lexer, Arena *arena) {
    _parser_expression(lexer, arena, 0);
}

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

ExprNode *parser_eval_expr(ExprNode *expr, Arena *a) {
    switch (expr->type) {
    case P_NUMBER:
        return expr;
    case P_IDENTIFIER:
        return expr;
    case P_ASSIGN: {
        ExprNode *left = parser_eval_expr(expr->left, a);
        ExprNode *right = parser_eval_expr(expr->right, a);
        return node_assign(a, left, right);
    }
    case P_PLUS:
    case P_DIV:
    case P_MULT: {
        ExprNode *left = parser_eval_expr(expr->left, a);
        assert(left && "Unexpected left null");
        assert(left->type == P_NUMBER && "Left is not a number");
        ExprNode *right = parser_eval_expr(expr->right, a);
        assert(right && "Unexpected right null");
        assert(right->type == P_NUMBER && "Right is not a number");
        double result =
            eval_number(expr->type, left->number_value, right->number_value);
        return node_number(a, result);
    }
    case P_MINUS: {
        ExprNode *result = parser_eval_expr(expr->right, a);
        assert(result && "Unexpected minus result null");
        assert(result->type == P_NUMBER && "Minus result is not a number");
        return node_number(a, (-1) * result->number_value);
    }
    }
    return NULL;
}

void _print_expression(ExprNode *expr, int depth) {
    for (size_t i = 0; i < depth; i++)
        printf("    ");
    switch (expr->type) {
    case P_NUMBER:
        printf("NUMBER(%lf)", expr->number_value);
        break;
    case P_IDENTIFIER:
        printf("ID(%s)", expr->string_value);
        break;
    case P_ASSIGN:
        printf("ASSIGN(\n");
        _print_expression(expr->left, depth + 1);
        printf("\n");
        for (size_t i = 0; i < depth + 1; i++)
            printf("    ");
        printf(",\n");
        _print_expression(expr->right, depth + 1);
        for (size_t i = 0; i < depth; i++)
            printf("    ");
        printf(")\n");
        break;
    case P_PLUS:
        printf("SUM(\n");
        _print_expression(expr->left, depth + 1);
        printf("\n");
        for (size_t i = 0; i < depth + 1; i++)
            printf("    ");
        printf(",\n");
        _print_expression(expr->right, depth + 1);
        printf(")");
        break;
    case P_MULT:
        printf("MULT(\n");
        _print_expression(expr->left, depth + 1);
        printf("\n");
        for (size_t i = 0; i < depth + 1; i++)
            printf("    ");
        printf(",\n");
        _print_expression(expr->right, depth + 1);
        printf(")");
        break;
    case P_MINUS:
        printf("MINUS(\n");
        _print_expression(expr->right, depth + 1);
        printf(")");
        break;
    case P_DIV:
        printf("DIV(\n");
        _print_expression(expr->left, depth + 1);
        printf("\n");
        for (size_t i = 0; i < depth + 1; i++)
            printf("    ");
        printf(",\n");
        _print_expression(expr->right, depth + 1);
        printf(")");
        break;
    default:
        assert(0 && "Not implemented expr type");
    }
}
void parser_print_expression(ExprNode *expr) {
    _print_expression(expr, 0);
    printf("\n");
}
#endif