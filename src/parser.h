#ifndef __PARSER_H_INCLUDED__
#define __PARSER_H_INCLUDED__

#include "lexer.h"
#include "utils/arena.h"

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
    P_FUNCALL,
} ExprType;

typedef struct op_node_t {
    ExprType type;

    double number_value;
    char char_value;
    char *string_value;
    struct op_node_t *left;
    struct op_node_t *right;

    struct op_node_t **items;
    int capacity;
    int count;
} ExprNode;

ExprNode *parser_expression(Lexer *lexer, Arena *arena);
void parser_print_expression(ExprNode *expr);
ExprNode *parser_eval_expr(ExprNode *expr, Arena *a);

ExprNode *node_number(Arena *a, double value);
ExprNode *node_minus(Arena *a, ExprNode *right);
ExprNode *node_op(Arena *a, TokenType op, ExprNode *left, ExprNode *right);
ExprNode *node_identifier(Arena *a, const char *value);

#endif // __PARSER_H_INCLUDED__

#ifndef __PARSER_H_IMP__
#define __PARSER_H_IMP__

#include "utils/array.h"

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

ExprNode *node_minus(Arena *a, ExprNode *right) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_MINUS;
    node->left = NULL;
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

ExprNode *node_string(Arena *a, const char *value) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_STRING;
    node->string_value = arena_strdup(a, value);
    return node;
}

ExprNode *node_funcall(Arena *a, const char *identifier) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->string_value = arena_strdup(a, identifier);
    node->capacity = 0;
    node->items = NULL;
    node->count = 0;
    node->type = P_FUNCALL;
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
    case T_ASSIGN:
        return node_assign(a, left, right);
    default:
        assert(0 && "Unexpected op node type");
    }
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
    if (!lexer_next_token(lexer)) return NULL;

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
    case T_STRING:
        return node_string(arena, lexer->token.string_value);
    case T_IDENTIFIER: {
        if (lexer_peek_next_char(lexer) != '(') {
            return node_identifier(arena, lexer->token.identifier_value);
        }

        ExprNode *funcall = node_funcall(arena, lexer->token.identifier_value);
        lexer_expect_token(lexer, T_OPAREN);
        while (lexer_peek_next_char(lexer) != ')') {
            ExprNode *arg = _parser_expression(lexer, arena, 0);
            if (arg == NULL) break;
            da_append(funcall, arg);
        }
        lexer_expect_token(lexer, T_CPAREN);
        return funcall;
    }
    default:
        LEXER_ERROR_PRINT(lexer, "Unexpected unop type: %d\n", lexer->token.type);
        exit(1);
    }
}

ExprNode *_parser_expression(Lexer *lexer, Arena *arena, int power) {
    ExprNode *left = _parse_unop(lexer, arena);
    if (!left) return left;

    Reader r = lexer_save_reader(lexer);

    while (1) {
        r = lexer_save_reader(lexer);
        if (!lexer_next_token(lexer)) return left;

        switch (lexer->token.type) {
        case T_PLUS:
        case T_MINUS:
        case T_MULT:
        case T_DIV: {
            char op = lexer->token.type;
            int op_power = get_infix_power(op);

            if (op_power <= power) goto rewind;
            ExprNode *right = _parser_expression(lexer, arena, op_power);
            if (!right) {
                LEXER_ERROR_PRINT(lexer, "Unexpected token after %c\n", op);
                exit(1);
            }
            left = node_op(arena, op, left, right);
        } break;
        case T_ASSIGN: {
            if (left->type != P_IDENTIFIER) {
                LEXER_ERROR_PRINT(lexer, "Token left than assign must be a identifier: %d\n", left->type);
                exit(1);
            }
            ExprNode *right = _parser_expression(lexer, arena, get_infix_power(T_ASSIGN));
            if (!right) {
                LEXER_ERROR_PRINT(lexer, "Unexpected token to assign\n");
                exit(1);
            }
            return node_assign(arena, left, right);
        } break;
        case T_NEW_LINE:
        case T_CPAREN:
            goto rewind;
        default:
            LEXER_ERROR_PRINT(lexer, "Unexpected token: %d\n", lexer->token.type);
            assert(0 && "unexpected token");
        }
    }
rewind:
    lexer_rewind_reader(lexer, r);
    return left;
}

ExprNode *parser_expression(Lexer *lexer, Arena *arena) { _parser_expression(lexer, arena, 0); }

void print_ws(int depth) {
    for (size_t i = 0; i < depth; i++)
        printf("        ");
}

void _print_expression(ExprNode *expr, int depth) {
    for (size_t i = 0; i < depth; i++)
        printf("        ");
    switch (expr->type) {
    case P_NUMBER:
        printf("NUMBER(%lf)", expr->number_value);
        break;
    case P_IDENTIFIER:
        printf("ID(%s)", expr->string_value);
        break;
    case P_FUNCALL:
        printf("FUNCALL(%s,\n", expr->string_value);
        for (size_t i = 0; i < expr->count; i++) {
            print_ws(depth);
            if (i > 0) printf(",\n");
            _print_expression(expr->items[i], depth + 1);
        }
        printf(")\n");
        break;
    case P_STRING:
        printf("STRING(%s)", expr->string_value);
        break;
    case P_ASSIGN:
        printf("ASSIGN(\n");
        _print_expression(expr->left, depth + 1);
        printf("\n");
        for (size_t i = 0; i < depth + 1; i++)
            printf("        ");
        printf(",\n");
        _print_expression(expr->right, depth + 1);
        for (size_t i = 0; i < depth; i++)
            printf("        ");
        printf(")\n");
        break;
    case P_PLUS:
        printf("SUM(\n");
        _print_expression(expr->left, depth + 1);
        printf("\n");
        for (size_t i = 0; i < depth + 1; i++)
            printf("        ");
        printf(",\n");
        _print_expression(expr->right, depth + 1);
        printf(")");
        break;
    case P_MULT:
        printf("MULT(\n");
        _print_expression(expr->left, depth + 1);
        printf("\n");
        for (size_t i = 0; i < depth + 1; i++)
            printf("        ");
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
            printf("        ");
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