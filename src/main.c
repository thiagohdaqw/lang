#include <stdio.h>
#include <stdlib.h>

#include "arena.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "[MAIN]: No file path specified\n");
        fprintf(stderr, "[MAIN]: Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    Lexer lexer = {0};
    if (!lexer_create(&lexer, argv[1])) {
        return 1;
    }

    Arena arena = arena_create(4 * 1024);

    while (1) {
        ExprNode *expr = parser_expression(&lexer, &arena);
        if (expr == NULL) {
            if (lexer_is_eof(&lexer))
                break;
            continue;
        }
        parser_print_expression(expr);

        printf("Result: ");
        ExprNode *result = parser_eval_expr(expr, &arena);
        assert(result && "Unexpected result null");
        parser_print_expression(result);
    }

    arena_destroy(&arena);

    return 0;
}