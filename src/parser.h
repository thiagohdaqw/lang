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
    P_PLUS,
    P_MINUS,
    P_IDENTIFIER,
    P_DIV,
    P_EQUAL,
    P_LESS,
    P_LEQUAL,
    P_GREATER,
    P_GEQUAL,
    P_AND,
    P_OR,
    P_MULT,
    P_POW,
    P_FUNC,
    P_FUNC_CALL,
    P_BLOCK,
    P_IF,
    P_WHILE,
    P_RETURN,
    P_INDEX,
    P_ARRAY,
    P_DEREF,
} ExprType;

typedef struct op_node_t ExprNode;

typedef struct func_args_t {
    // TODO: USE DA IN ARENA
    ExprNode **items;
    int capacity;
    int count;
} FunArgs;

typedef struct op_node_t {
    ExprType type;

    double number_value;
    int length;
    char char_value;
    char *string_value;

    bool returned;

    ExprNode *first;
    ExprNode *second;
    ExprNode *third;

    FunArgs args;

    // TODO: USE DA IN ARENA
    ExprNode **items;
    int capacity;
    int count;
} ExprNode;

ExprNode *parser_parse_expression(Lexer *lexer, Arena *arena);
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
    node->first = left;
    node->second = right;
    return node;
}

ExprNode *node_minus(Arena *a, ExprNode *value) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_MINUS;
    node->first = value;
    return node;
}

ExprNode *node_mult(Arena *a, ExprNode *left, ExprNode *right) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_MULT;
    node->first = left;
    node->second = right;
    return node;
}

ExprNode *node_div(Arena *a, ExprNode *left, ExprNode *right) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_DIV;
    node->first = left;
    node->second = right;
    return node;
}

ExprNode *node_pow(Arena *a, ExprNode *left, ExprNode *right) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_POW;
    node->first = left;
    node->second = right;
    return node;
}

ExprNode *node_binop(Arena *a, TokenType type, ExprNode *left, ExprNode *right) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = type;
    node->first = left;
    node->second = right;
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
    node->first = left;
    node->second = right;
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
    node->args.items = NULL;
    node->args.capacity = 0;
    node->args.count = 0;
    node->type = P_FUNC_CALL;
    return node;
}

ExprNode *node_array(Arena *a, ExprNode *id, ExprNode *index) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->first = id;
    node->second = index;
    node->type = P_ARRAY;
    return node;
}

ExprNode *node_index(Arena *a, ExprNode *address, ExprNode *index) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->first = address;
    node->second = index;
    node->type = P_INDEX;
    return node;
}

ExprNode *node_deref(Arena *a, ExprNode *value) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->first = value;
    node->type = P_DEREF;
    return node;
}

ExprNode *node_func(Arena *a, const char *identifier) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->string_value = arena_strdup(a, identifier);
    node->items = NULL;
    node->capacity = 0;
    node->count = 0;
    node->type = P_FUNC;
    return node;
}

ExprNode *node_block(Arena *a) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_BLOCK;
    node->items = NULL;
    node->count = 0;
    node->capacity = 0;
    return node;
}

ExprNode *node_if(Arena *a, ExprNode *condition_node, ExprNode *then_node, ExprNode *else_node) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_IF;
    node->first = condition_node;
    node->second = then_node;
    node->third = else_node;
    return node;
}

ExprNode *node_while(Arena *a, ExprNode *condition, ExprNode *body) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_WHILE;
    node->first = condition;
    node->second = body;
    return node;
}

ExprNode *node_return(Arena *a, ExprNode *return_value) {
    ExprNode *node = (ExprNode *)arena_alloc(a, sizeof(*node));
    node->type = P_RETURN;
    node->first = return_value;
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
    case T_POW:
        return node_pow(a, left, right);
    case T_ASSIGN:
        return node_assign(a, left, right);
    case T_AND:
        return node_binop(a, P_AND, left, right);
    case T_OR:
        return node_binop(a, P_OR, left, right);
    case T_EQUAL:
        return node_binop(a, P_EQUAL, left, right);
    case T_LESS:
        return node_binop(a, P_LESS, left, right);
    case T_LEQUAL:
        return node_binop(a, P_LEQUAL, left, right);
    case T_GREATER:
        return node_binop(a, P_GREATER, left, right);
    case T_GEQUAL:
        return node_binop(a, P_GEQUAL, left, right);
    default:
        fprintf(stderr, "Unexpected op node type: %d\n", op);
        assert(0 && "Unexpected op node type");
    }
}

static int get_infix_power(TokenType type) {
    switch (type) {
    case T_POW:
        return 70;
    case T_MULT:
    case T_DIV:
        return 60;
    case T_PLUS:
    case T_MINUS:
        return 50;
    case T_EQUAL:
    case T_LESS:
    case T_LEQUAL:
    case T_GREATER:
    case T_GEQUAL:
        return 40;
    case T_AND:
        return 30;
    case T_OR:
        return 20;
    case T_DEREF:
        return 10;
    case T_ASSIGN:
        return 0;
    default:
        return 0;
    }
}

static bool is_infix_right_associative(TokenType type) {
    switch (type) {
    case T_POW:
        return true;
    default:
        return false;
    }
}

static ExprNode *_parser_expression(Lexer *lexer, Arena *arena, int min_power);

static ExprNode *_parse_identifier(Lexer *lexer, Arena *arena);

static ExprNode *_parse_funcall(Arena *arena, Lexer *lexer);

static ExprNode *_parse_func(Lexer *lexer, Arena *arena);

static ExprNode *_parse_if(Lexer *lexer, Arena *arena);

static ExprNode *_parse_while(Lexer *lexer, Arena *arena);

static ExprNode *_parse_return(Lexer *lexer, Arena *arena);

static ExprNode *_parse_prefix(Lexer *lexer, Arena *arena) {
    if (!lexer_next_token(lexer)) return NULL;

    switch (lexer->token.type) {
    case T_NEW_LINE:
    case T_ELSE:
    case T_END:
        return NULL;
    case T_LITERAL:
        switch (lexer->token.literal_value) {
        case '-':
            return node_minus(arena, _parser_expression(lexer, arena, 70));
        default:
            return NULL;
        }
    case T_LONG:
        return node_number(arena, (double)lexer->token.long_value);
    case T_DOUBLE:
        return node_number(arena, lexer->token.double_value);
    case T_STRING:
        return node_string(arena, lexer->token.string_value);
    case T_IDENTIFIER:
        return _parse_identifier(lexer, arena);
    case T_FUNC:
        return _parse_func(lexer, arena);
    case T_IF:
        return _parse_if(lexer, arena);
    case T_WHILE:
        return _parse_while(lexer, arena);
    case T_RETURN:
        return _parse_return(lexer, arena);
    case T_MINUS:
        return node_minus(arena, _parser_expression(lexer, arena, 0));
    case T_OPAREN: {
        ExprNode *value = parser_parse_expression(lexer, arena);
        lexer_expect_token(lexer, T_CPAREN);
        return value;
    }
    case T_MULT: {
        ExprNode *value = _parser_expression(lexer, arena, get_infix_power(T_DEREF));
        return node_deref(arena, value);
    }
    default:
        LEXER_ERROR_PRINT(lexer, "Unexpected unop type: %d\n", lexer->token.type);
        assert(0 && "Unexpected unop");
    }
}

static ExprNode *_parser_expression(Lexer *lexer, Arena *arena, int power) {
    ExprNode *left = _parse_prefix(lexer, arena);
    if (!left) return left;

    Reader r = lexer_save_reader(lexer);

    while (1) {
        r = lexer_save_reader(lexer);
        if (!lexer_next_token(lexer)) return left;

        switch (lexer->token.type) {
        case T_EQUAL:
        case T_AND:
        case T_OR:
        case T_LESS:
        case T_LEQUAL:
        case T_GREATER:
        case T_GEQUAL:
        case T_PLUS:
        case T_MINUS:
        case T_MULT:
        case T_DIV:
        case T_POW: {
            char op = lexer->token.type;
            int op_power = get_infix_power(op);

            if (op_power <= power) goto rewind;

            if (is_infix_right_associative(op)) {
                op_power -= 1;
            }

            ExprNode *right = _parser_expression(lexer, arena, op_power);
            if (!right) {
                LEXER_ERROR_PRINT(lexer, "Unexpected token after %c\n", op);
                exit(1);
            }
            left = node_op(arena, op, left, right);
        } break;
        case T_ASSIGN: {
            if (get_infix_power(T_ASSIGN) < power) goto rewind;

            if (left->type != P_IDENTIFIER && left->type != P_DEREF && left->type != P_INDEX) {
                LEXER_ERROR_PRINT(lexer, "Token left than assign must be a identifier or deref or index: %d\n",
                                  left->type);
                assert(0 && "Token left than assign must be a identifier or deref or index");
            }
            ExprNode *right = _parser_expression(lexer, arena, get_infix_power(T_ASSIGN));
            if (!right) {
                LEXER_ERROR_PRINT(lexer, "Unexpected token to assign\n");
                exit(1);
            }
            return node_assign(arena, left, right);
        }
        case T_LITERAL:
            switch (lexer->token.literal_value) {
            case '[':
                ExprNode *index = parser_parse_expression(lexer, arena);
                lexer_expect_literal(lexer, ']');
                left = node_index(arena, left, index);
                break;
            default:
                goto rewind;
            }
            break;
        default:
            goto rewind;
        }
    }
rewind:
    lexer_rewind_reader(lexer, r);
    return left;
}

ExprNode *parser_parse_expression(Lexer *lexer, Arena *arena) { _parser_expression(lexer, arena, 0); }

static ExprNode *_parse_block(Lexer *lexer, Arena *arena, TokenType end) {
    ExprNode *block = node_block(arena);
    while (1) {
        Reader r = lexer_save_reader(lexer);
        ExprNode *node = parser_parse_expression(lexer, arena);
        if (node == NULL) {
            if (lexer->token.type == end || lexer->token.type == T_END) {
                lexer_rewind_reader(lexer, r);
                return block;
            }
            if (lexer->token.type == T_NEW_LINE) {
                continue;
            }
            LEXER_ERROR_PRINT(lexer, "Unexpected end of block: %d\n", lexer->token.type);
            assert(0 && "Unexpected end of block");
        }
        da_append(block, node);
    }
    assert(0 && "Unreachable");
}

static ExprNode *_parse_func(Lexer *lexer, Arena *arena) {
    lexer_expect_token(lexer, T_IDENTIFIER);
    ExprNode *func = node_func(arena, lexer->token.identifier_value);
    lexer_expect_token(lexer, T_OPAREN);

    while (lexer_peek_next_char(lexer) != ')') {
        lexer_expect_token(lexer, T_IDENTIFIER);
        ExprNode *arg = node_identifier(arena, lexer->token.identifier_value);
        da_append(&func->args, arg);

        if (lexer_peek_next_char(lexer) != ',') {
            break;
        }
        lexer_expect_literal(lexer, ',');
    }
    lexer_expect_token(lexer, T_CPAREN);
    lexer_expect_literal(lexer, ':');

    func->first = _parse_block(lexer, arena, T_END);

    lexer_expect_token(lexer, T_END);
    return func;
}

static ExprNode *_parse_identifier(Lexer *lexer, Arena *arena) {
    switch (lexer_peek_next_char(lexer)) {
    case '(':
        return _parse_funcall(arena, lexer);
    case '[': {
        Reader reader = lexer_save_reader(lexer);

        lexer_next_token(lexer);
        ExprNode *id = node_identifier(arena, lexer->token.identifier_value);
        ExprNode *index = parser_parse_expression(lexer, arena);
        lexer_expect_literal(lexer, ']');

        Token next = lexer_peek_next_token(lexer);
        if (next.type == T_LITERAL && next.literal_value == ';') {
            return node_array(arena, id, index);
        } else {
            lexer_rewind_reader(lexer, reader);
            return node_identifier(arena, lexer->token.identifier_value);
        }
    } break;
    default:
        return node_identifier(arena, lexer->token.identifier_value);
    }
}

ExprNode *_parse_funcall(Arena *arena, Lexer *lexer) {
    ExprNode *funcall = node_funcall(arena, lexer->token.identifier_value);
    lexer_expect_token(lexer, T_OPAREN);
    while (lexer_peek_next_char(lexer) != ')') {
        ExprNode *arg = _parser_expression(lexer, arena, 0);
        if (arg == NULL) break;
        da_append(&funcall->args, arg);

        if (lexer_peek_next_char(lexer) != ',') {
            break;
        }
        lexer_expect_literal(lexer, ',');
    }
    lexer_expect_token(lexer, T_CPAREN);
    return funcall;
}

static ExprNode *_parse_if(Lexer *lexer, Arena *arena) {
    ExprNode *condition_node = parser_parse_expression(lexer, arena);

    lexer_expect_literal(lexer, ':');

    ExprNode *then_node = _parse_block(lexer, arena, T_ELSE);

    ExprNode *else_node = NULL;
    if (lexer_peek_next_token(lexer).type == T_ELSE) {
        lexer_next_token(lexer);
        else_node = _parse_block(lexer, arena, T_END);
    }

    lexer_expect_token(lexer, T_END);

    return node_if(arena, condition_node, then_node, else_node);
}

static ExprNode *_parse_while(Lexer *lexer, Arena *arena) {
    ExprNode *condition = parser_parse_expression(lexer, arena);

    lexer_expect_literal(lexer, ':');

    ExprNode *body = _parse_block(lexer, arena, T_END);

    lexer_expect_token(lexer, T_END);
    return node_while(arena, condition, body);
}

static ExprNode *_parse_return(Lexer *lexer, Arena *arena) {
    ExprNode *ret = parser_parse_expression(lexer, arena);
    return node_return(arena, ret);
}

static void print_ws(int depth) {
    for (size_t i = 0; i < depth; i++)
        printf("  ");
}

static void _print_expression(ExprNode *expr, int depth) {
    if (!expr) return;
    print_ws(depth);
    switch (expr->type) {
    case P_NUMBER:
        printf("NUMBER(%lf)", expr->number_value);
        break;
    case P_IDENTIFIER:
        printf("ID(%s)", expr->string_value);
        break;
    case P_FUNC_CALL:
        printf("FUNCALL(%s,\n", expr->string_value);
        for (size_t i = 0; i < expr->args.count; i++) {
            print_ws(depth);
            if (i > 0) printf(",\n");
            _print_expression(expr->args.items[i], depth + 1);
        }
        printf(")\n");
        break;
    case P_STRING:
        printf("STRING(%s)", expr->string_value);
        break;
    case P_ASSIGN:
        printf("ASSIGN(\n");
        _print_expression(expr->first, depth + 1);
        printf("\n");
        print_ws(depth + 1);
        printf(",\n");
        _print_expression(expr->second, depth + 1);
        print_ws(depth);
        printf(")\n");
        break;
    case P_PLUS:
        printf("SUM(\n");
        _print_expression(expr->first, depth + 1);
        printf("\n");
        print_ws(depth + 1);
        printf(",\n");
        _print_expression(expr->second, depth + 1);
        printf(")");
        break;
    case P_MULT:
        printf("MULT(\n");
        _print_expression(expr->first, depth + 1);
        printf("\n");
        print_ws(depth + 1);
        printf(",\n");
        _print_expression(expr->second, depth + 1);
        printf(")");
        break;
    case P_MINUS:
        printf("MINUS(\n");
        _print_expression(expr->first, depth + 1);
        printf(")");
        break;
    case P_DIV:
        printf("DIV(\n");
        _print_expression(expr->first, depth + 1);
        printf("\n");
        print_ws(depth + 1);
        printf(",\n");
        _print_expression(expr->second, depth + 1);
        printf(")");
        break;
    case P_FUNC:
        printf("FUNC(%s, args = %d, body = %d)\n", expr->string_value, expr->args.count, expr->count);
        break;
    case P_DEREF:
        printf("DREF(\n");
        _print_expression(expr->first, depth + 1);
        printf(")");
        break;
    case P_RETURN:
        printf("RETURN(\n");
        _print_expression(expr->first, depth + 1);
        printf(")");
        break;
    case P_INDEX:
        printf("INDEX(\n");
        _print_expression(expr->first, depth + 1);
        printf("\n");
        print_ws(depth + 1);
        printf(",\n");
        _print_expression(expr->second, depth + 1);
        printf(")");
        break;
    case P_ARRAY:
        printf("ARRAY(\n");
        _print_expression(expr->first, depth + 1);
        printf("\n");
        print_ws(depth + 1);
        printf(",\n");
        _print_expression(expr->second, depth + 1);
        printf(")");
        break;
    case P_IF:
        printf("IF(\n");
        printf(")");
        break;
    default:
        fprintf(stderr, "Not imeplemnted expr type: %d\n", expr->type);
        assert(0 && "Not implemented expr type");
    }
}

void parser_print_expression(ExprNode *expr) {
    if (!expr) return;
    _print_expression(expr, 0);
    printf("\n");
}
#endif