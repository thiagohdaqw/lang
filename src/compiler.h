#ifndef __COMPILER_H_INCLUDED__
#define __COMPILER_H_INCLUDED__

#include "asm_compiler.h"
#include "parser.h"

typedef enum {
    NONE,
    ASM,
} CompilerTypes;

typedef struct {
    CompilerTypes type;
    AsmCompiler asm_compiler;

    Arena allocator;
    const char *build_folder;
    const char *entry_path;
    const char *output_executable_path;
    const char *c_args;
    const char *linker_args;
} Compiler;

Compiler compiler_create(CompilerTypes type, const char *build_folder, const char *output_executable_path,
                         const char *c_args, const char *linker_args);
void compiler_destroy(Compiler *c);
bool compiler_init(Compiler *c);
void compiler_append_expression(Compiler *c, ExprNode *expr);
void compiler_compile(Compiler *c);

#endif // __COMPILER_H_INCLUDED__

#ifndef __COMPILER_H_IMP__
#define __COMPILER_H_IMP__

#include <assert.h>
#include <stdio.h>

Compiler compiler_create(CompilerTypes type, const char *build_folder, const char *output_executable_path,
                         const char *c_args, const char *linker_args) {
    Compiler c = {0};

    c.type = type;
    c.build_folder = build_folder;
    c.output_executable_path = output_executable_path;
    c.c_args = c_args;
    c.linker_args = linker_args;
    c.allocator = arena_create(4 * 1024);

    switch (type) {
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
    switch (c->type) {
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

    c->entry_path = arena_strjoin(&c->allocator, c->build_folder, "entry.c");
    FILE *entry = file_create(c->entry_path);

    file_write(entry, c_entry);

    file_close(entry);
}

bool compiler_init(Compiler *c) {
    _create_c_entry(c);

    switch (c->type) {
    case ASM:
        return asm_compiler_init(&c->asm_compiler, c->build_folder);
        break;
    default:
        assert(0 && "Compiler type not implemented");
    }
}

void compiler_append_expression(Compiler *c, ExprNode *expr) {
    switch (c->type) {
    case ASM:
        asm_compiler_append_expression(&c->asm_compiler, expr);
        break;
    default:
        assert(0 && "Compiler type not implemented");
    }
}

void compiler_compile(Compiler *c) {
    ArenaNode saved = arena_save(&c->allocator);

    const char *output_object_path = arena_strjoin(&c->allocator, c->build_folder, "out.o");

    switch (c->type) {
    case ASM:
        asm_compiler_generate_assembly(&c->asm_compiler);
        if (!asm_compiler_compile(&c->asm_compiler, output_object_path)) {
            return;
        }
        break;
    default:
        assert(0 && "Compiler type not implemented");
    }

    char *compile_command = arena_strformat(&c->allocator,
        "gcc -no-pie %s %s %s -o %s %s",
        c->c_args,
        c->entry_path,
        output_object_path,
        c->output_executable_path,
        c->linker_args
    );

    printf("Executing: %s\n", compile_command);
    int ret = system(compile_command);
    printf("Returned: %d\n", ret);

    arena_rewind(&c->allocator, saved);
}

#endif