#include <stdio.h>
#include <stdlib.h>

#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "utils/arena.h"
#define STB_DS_IMPLEMENTATION
#include "compiler.h"
#include "utils/file.h"
#include "utils/stb_ds.h"

int interpret(int argc, char** argv) {
    Lexer lexer = { 0 };
    if (!lexer_init(&lexer, argv[1])) {
        return 1;
    }

    Arena arena = arena_create(4 * 1024);
    Arena temp_arena = arena_create(4 * 1024);

    Interpreter interpreter = interpreter_create(&arena, &temp_arena);

    while (1) {
        ExprNode* expr = parser_expression(&lexer, &arena);
        if (expr == NULL) {
            if (lexer_is_eof(&lexer)) break;
            continue;
        }
        interpreter_append(&interpreter, expr);
    }

    int ret = interpreter_eval(&interpreter);

    arena_destroy(&arena);

    return ret;
}

int compile(int argc, char** argv) {
    const char* build_folder = "./.pypt_build/";
    const char* executable_output_path = "main";
    const char* compiler_c_args = "";
    const char* compiler_linker_args = "-lm";
    CompilerTypes compiler_type = FX86;

    if (!folder_create(build_folder, 0755)) {
        return 1;
    }

    Compiler compiler = compiler_create(compiler_type, build_folder, executable_output_path, compiler_c_args, compiler_linker_args);

    if (!compiler_init(&compiler)) {
        return 1;
    }

    Lexer lexer = { 0 };
    if (!lexer_init(&lexer, argv[1])) {
        return 1;
    }

    Arena arena = arena_create(4 * 1024);

    while (1) {
        ExprNode* expr = parser_expression(&lexer, &arena);
        if (expr == NULL) {
            if (lexer_is_eof(&lexer)) break;
            continue;
        }
        compiler_append_expression(&compiler, expr);
    }

    compiler_compile(&compiler);

    arena_destroy(&arena);
    compiler_destroy(&compiler);

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "[MAIN]: No file path specified\n");
        fprintf(stderr, "[MAIN]: Usage: %s <file_path>\n", argv[0]);
        return 1;
    }
    if (0) {
        return interpret(argc, argv);
    }
    if (1) {
        return compile(argc, argv);
    }
}