#ifndef __COMPILER_H_INCLUDED__
#define __COMPILER_H_INCLUDED__

#include "asm_compiler.h"
#include "parser.h"

typedef enum {
    NONE,
    ASM,
} CompilerTarget;

typedef struct {
    CompilerTarget target;
    const char *build_folder;
    const char *entry_path;
    const char *output;
    bool output_object;
    const char *c_args;
    const char *linker_args;
} CompilerConfig;

typedef struct {
    AsmCompiler asm_compiler;

    Arena allocator;

    CompilerConfig config;
} Compiler;

void compiler_parse_args(CompilerConfig *config, int argc, char **argv);
Compiler compiler_create(CompilerConfig config);
void compiler_destroy(Compiler *c);
bool compiler_init(Compiler *c);
void compiler_append_expression(Compiler *c, ExprNode *expr);
void compiler_compile(Compiler *c);

#endif // __COMPILER_H_INCLUDED__

#ifndef __COMPILER_H_IMP__
#define __COMPILER_H_IMP__

#include <assert.h>
#include <stdio.h>

Compiler compiler_create(CompilerConfig config) {
    Compiler c = {0};

    c.config = config;
    c.allocator = arena_create(4 * 1024);

    switch (c.config.target) {
    case ASM:
        c.asm_compiler = asm_compiler_create();
        break;
    default:
        assert(0 && "Compiler type not implemented");
    }

    return c;
}

void compiler_destroy(Compiler *c) {
    arena_destroy(&c->allocator);
    switch (c->config.target) {
    case ASM:
        asm_compiler_destroy(&c->asm_compiler);
        break;
    default:
        assert(0 && "Compiler type not implemented");
    }
}

static void _create_c_entry(Compiler *c) {
    const char *c_entry = "extern int pypt_main(int argc, char **argv);\n"
                          "int main(int argc, char **argv) {\n"
                          "    return pypt_main(argc, argv);\n"
                          "}\n";

    c->config.entry_path = arena_strjoin(&c->allocator, c->config.build_folder, "entry.c");
    FILE *entry = file_create(c->config.entry_path);

    file_write(entry, c_entry);

    file_close(entry);
}

bool compiler_init(Compiler *c) {
    _create_c_entry(c);

    switch (c->config.target) {
    case ASM:
        return asm_compiler_init(&c->asm_compiler, c->config.build_folder, c->config.output_object);
        break;
    default:
        assert(0 && "Compiler type not implemented");
    }
}

void compiler_append_expression(Compiler *c, ExprNode *expr) {
    switch (c->config.target) {
    case ASM:
        asm_compiler_append_expression(&c->asm_compiler, expr);
        break;
    default:
        assert(0 && "Compiler type not implemented");
    }
}

void compiler_compile(Compiler *c) {
    ArenaNode saved = arena_save(&c->allocator);

    const char *output_object_path = arena_strjoin(&c->allocator, c->config.build_folder, "out.o");

    switch (c->config.target) {
    case ASM:
        asm_compiler_generate_assembly(&c->asm_compiler, !c->config.output_object);
        if (!asm_compiler_compile(&c->asm_compiler, output_object_path)) {
            return;
        }
        break;
    default:
        assert(0 && "Compiler type not implemented");
    }

    if (!c->config.output_object) {
        char *stdlib = arena_strformat(&c->allocator, "%sstdlib/*.o", c->config.build_folder);
        char *compile_command =
            arena_strformat(&c->allocator, "gcc -no-pie %s %s %s %s -o %s %s",
                c->config.c_args,
                stdlib,
                c->config.entry_path,
                output_object_path,
                c->config.output,
                c->config.linker_args);

        printf("Executing: %s\n", compile_command);
        int ret = system(compile_command);
        printf("Returned: %d\n", ret);
    }

    arena_rewind(&c->allocator, saved);
}

void compiler_parse_args(CompilerConfig *config, int argc, char **argv) {
    for (size_t i = 2; i < argc; i++) {
        char *arg = argv[i];
        if (strcmp("-o", arg) == 0 && i + 1 < argc) {
            config->output = argv[i++];
        } else if (strcmp("-C", arg) == 0) {
            config->output_object = true;
        }
    }
}

#endif