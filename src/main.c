#include <stdio.h>
#include <stdlib.h>

#include "utils/arena.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "[MAIN]: No file path specified\n");
        fprintf(stderr, "[MAIN]: Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    Lexer lexer = {0};
    if (!lexer_init(&lexer, argv[1])) {
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

        printf("\n====================================\n");
        parser_print_expression(expr);

        printf("Result:\n");
        ExprNode *result = interpreter_eval(expr, &arena);
        if (!result) continue;
        parser_print_expression(result);

    }

    arena_destroy(&arena);

    return 0;
}