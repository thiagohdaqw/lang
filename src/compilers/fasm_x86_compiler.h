#ifndef __FASM_X86_COMPILER_H_INCLUDED__
#define __FASM_X86_COMPILER_H_INCLUDED__

#include "../utils/arena.h"
#include "../parser.h"

typedef struct {
    ExprNode *expr;
    long identifier;
} CNode;

typedef struct {
    CNode** items;
    int capacity;
    int count;
} CBlock;

typedef struct {
    CNode** items;
    int capacity;
    int count;
} CData;

typedef struct {
    Arena allocator;

    const char* asm_file_path;
    FILE* asm_file;

    CBlock main;

    CData data;
} FX86Compiler;

FX86Compiler fasm_x86_compiler_create();
bool fasm_x86_compiler_init(FX86Compiler* c, const char* build_folder);
void fasm_x86_compiler_destroy(FX86Compiler* c);
void fasm_x86_compiler_append_expression(FX86Compiler* c, ExprNode* expr);
void fasm_x86_compiler_generate_assembly(FX86Compiler* c);
bool fasm_x86_compiler_compile(FX86Compiler *c, const char *object_output_path);

#endif // __FASM_X86_COMPILER_H_INCLUDED__

#ifndef __FASM_X86_COMPILER_H_IMP__
#define __FASM_X86_COMPILER_H_IMP__

FX86Compiler fasm_x86_compiler_create() {
    FX86Compiler compiler = { 0 };
    compiler.allocator = arena_create(4 * 1024);
    return compiler;
}

void fasm_x86_compiler_destroy(FX86Compiler* c) { arena_destroy(&c->allocator); }

bool fasm_x86_compiler_init(FX86Compiler* c, const char* build_folder) {
    c->asm_file_path = arena_strjoin(&c->allocator, build_folder, "fasm_x86_out.asm");
    c->asm_file = file_create(c->asm_file_path);

    const char* fasm_entry =
        "format ELF64\n"
        "public pypt_entry\n\n"
        "section '.text' executable align 16\n";

    file_write(c->asm_file, fasm_entry);
    return true;
}

void fasm_x86_compiler_append_expression(FX86Compiler* c, ExprNode* expr) {
    CNode *node = (CNode *)arena_alloc(&c->allocator, sizeof(*node));
    node->expr = expr;

    if (node->expr->type == P_STRING) {
        node->identifier = c->data.count + 1;
        da_append(&c->data, node);
    }

    da_append(&c->main, node);
}

void fasm_x86_compiler_generate_assembly(FX86Compiler* c) {
    const char *entry = "pypt_entry:\n";
    file_write(c->asm_file, entry);

    const char *entry_return =
        "    mov rax, 0\n"
        "    ret\n";
    file_write(c->asm_file, entry_return);

    fclose(c->asm_file);
}

bool fasm_x86_compiler_compile(FX86Compiler *c, const char *object_output_path) {
    ArenaNode saved = arena_save(&c->allocator);

    const char *fasm_command = arena_strjoin(&c->allocator, "fasm ", c->asm_file_path);
    fasm_command = arena_strjoin(&c->allocator, fasm_command, " ");
    fasm_command = arena_strjoin(&c->allocator, fasm_command, object_output_path);

    printf("Executing: %s\n", fasm_command);
    int ret = system(fasm_command);
    printf("Returned: %d\n", ret);

    arena_rewind(&c->allocator, saved);

    return ret == 0;
}

#endif