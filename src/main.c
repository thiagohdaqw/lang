#include <stdio.h>
#include <stdlib.h>

#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "utils/arena.h"
#define STB_DS_IMPLEMENTATION
#include "utils/stb_ds.h"

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
    Arena temp_arena = arena_create(4 * 1024);

    Interpreter interpreter = interpreter_create(&arena, &temp_arena);

    while (1) {
        ExprNode *expr = parser_expression(&lexer, &arena);
        if (expr == NULL) {
            if (lexer_is_eof(&lexer)) break;
            continue;
        }
        interpreter_append(&interpreter, expr);
    }

    interpreter_eval(&interpreter);

    arena_destroy(&arena);

    return 0;
}