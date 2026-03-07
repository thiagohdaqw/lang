#ifndef __FASM_X86_COMPILER_H_INCLUDED__
#define __FASM_X86_COMPILER_H_INCLUDED__

#include "../parser.h"
#include "../utils/arena.h"

typedef struct {
    ExprNode *expr;
    long identifier;
    const char *reg;
} CNode;

typedef struct {
    ExprNode **items;
    int capacity;
    int count;
} CBlock;

typedef struct {
    CNode **items;
    int capacity;
    int count;
} CData;

typedef struct {
    Arena allocator;

    const char *asm_file_path;
    FILE *asm_file;

    CBlock main;

    CData data;
} FX8664Compiler;

FX8664Compiler fasm_x86_64_compiler_create();
bool fasm_x86_64_compiler_init(FX8664Compiler *c, const char *build_folder);
void fasm_x86_64_compiler_destroy(FX8664Compiler *c);
void fasm_x86_64_compiler_append_expression(FX8664Compiler *c, ExprNode *expr);
void fasm_x86_64_compiler_generate_assembly(FX8664Compiler *c);
bool fasm_x86_64_compiler_compile(FX8664Compiler *c, const char *object_output_path);

#endif // __FASM_X86_COMPILER_H_INCLUDED__

#ifndef __FASM_X86_COMPILER_H_IMP__
#define __FASM_X86_COMPILER_H_IMP__

FX8664Compiler fasm_x86_64_compiler_create() {
    FX8664Compiler compiler = {0};
    compiler.allocator = arena_create(4 * 1024);
    return compiler;
}

void fasm_x86_64_compiler_destroy(FX8664Compiler *c) { arena_destroy(&c->allocator); }

bool fasm_x86_64_compiler_init(FX8664Compiler *c, const char *build_folder) {
    c->asm_file_path = arena_strjoin(&c->allocator, build_folder, "fasm_x86_out.asm");
    c->asm_file = file_create(c->asm_file_path);

    const char *fasm_entry = "format ELF64\n"
                             "public pypt_entry\n\n"
                             "section '.text' executable align 16\n";

    file_write(c->asm_file, fasm_entry);
    return true;
}

void fasm_x86_64_compiler_append_expression(FX8664Compiler *c, ExprNode *expr) { da_append(&c->main, expr); }

CNode *node_create(Arena *a, ExprNode *expr) {
    CNode *node = (CNode *)arena_alloc(a, sizeof(*node));
    node->expr = expr;
    return node;
}

CNode *_compile_expression(FX8664Compiler *c, CBlock *block, ExprNode *expr, Arena *a, int depth);

void fasm_x86_64_compiler_generate_assembly(FX8664Compiler *c) {
    const char *entry = "pypt_entry:\n";
    file_write(c->asm_file, entry);

    CNode *result = NULL;

    for (size_t i = 0; i < c->main.count; i++) {
        ArenaNode saved = arena_save(&c->allocator);

        result = _compile_expression(c, &c->main, c->main.items[i], &c->allocator, 1);

        arena_rewind(&c->allocator, saved);
    }

    if (result && result->expr->type != P_RETURN) {
        const char *entry_return = "    mov eax, 0\n"
                                   "    ret\n";
        file_write(c->asm_file, entry_return);
    }

    fclose(c->asm_file);
}

bool fasm_x86_64_compiler_compile(FX8664Compiler *c, const char *object_output_path) {
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

void write_spaces(FX8664Compiler *c, int depth) {
    for (size_t i = 0; i < depth; i++)
        file_write(c->asm_file, "    ");
}

#define asm_write(c, d, v)                                                                                             \
    write_spaces(c, d);                                                                                                \
    file_write((c)->asm_file, v);
#define asm_fwrite(c, a, d, f, ...)                                                                                    \
    write_spaces(c, d);                                                                                                \
    file_write((c)->asm_file, arena_strformat((a), f, ##__VA_ARGS__));

CNode *_compile_expression(FX8664Compiler *c, CBlock *block, ExprNode *expr, Arena *a, int depth) {
    switch (expr->type) {
    case P_NUMBER: {
        // TODO: add suport to float
        CNode *node = node_create(a, expr);
        node->reg = "rcx";
        asm_fwrite(c, a, depth, "mov %s, %d\n", node->reg, (int)expr->number_value);
        return node;
    }
    case P_RETURN: {
        CNode *child = _compile_expression(c, block, expr->first, a, depth);
        asm_fwrite(c, a, depth, "mov rax, %s\n", child->reg);
        asm_write(c, depth, "ret\n");
        return node_create(a, expr);
    }
    case P_MINUS: {
        CNode *child = _compile_expression(c, block, expr->first, a, depth);
        CNode *result = node_create(a, expr);
        result->reg = child->reg;
        asm_fwrite(c, a, depth, "neg %s\n", child->reg);
        return result;
    }
    case P_MULT: {
        CNode *first_child = _compile_expression(c, block, expr->first, a, depth);
        asm_fwrite(c, a, depth, "push %s\n", first_child->reg);
        CNode *second_child = _compile_expression(c, block, expr->second, a, depth);
        asm_fwrite(c, a, depth, "push %s\n", second_child->reg);
        asm_write(c, depth, "pop rcx\n");
        asm_write(c, depth, "pop rdx\n");
        asm_write(c, depth, "imul rcx, rdx\n");
        CNode *result = node_create(a, expr);
        result->reg = "rcx";
        return result;
    }
    case P_PLUS: {
        CNode *first_child = _compile_expression(c, block, expr->first, a, depth);
        asm_fwrite(c, a, depth, "push %s\n", first_child->reg);
        CNode *second_child = _compile_expression(c, block, expr->second, a, depth);
        asm_fwrite(c, a, depth, "push %s\n", second_child->reg);
        asm_write(c, depth, "pop rcx\n");
        asm_write(c, depth, "pop rdx\n");
        asm_write(c, depth, "add rcx, rdx\n");
        CNode *result = node_create(a, expr);
        result->reg = "rcx";
        return result;
    }
    case P_CHAR:
    case P_STRING:
    case P_ASSIGN:
    case P_BINOP:
    case P_IDENTIFIER:
    case P_DIV:
    case P_POW:
    case P_FUNC:
    case P_FUN_CALL:
    case P_BLOCK:
    case P_IF:
    case P_WHILE:
    default:
        fprintf(stderr, "Type not implemented: %d\n", expr->type);
        assert(0 && "Type not implemented");
    }
}

#endif