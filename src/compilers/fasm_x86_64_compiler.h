#ifndef __FASM_X86_COMPILER_H_INCLUDED__
#define __FASM_X86_COMPILER_H_INCLUDED__

#include "../parser.h"
#include "../utils/arena.h"

typedef struct {
    ExprNode *expr;
    size_t stack_index;
    const char *reg;
} CNode;

typedef struct {
    ExprNode **items;
    int capacity;
    int count;
} CBlock;

typedef struct {
    const char *key;
    CNode *value;
} CVarMap;

typedef struct {
    CVarMap *vars;
    int count;
} CScope;

typedef struct {
    CNode **items;
    int capacity;
    int count;
} CData;

typedef struct {
    Arena allocator;

    const char *asm_file_path;
    FILE *asm_file;

    CBlock funcs;
    CBlock main;
    CScope main_scope;

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

static const char *ARG_REGS[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

#define asm_write(c, d, v)                                                                                             \
    write_spaces((c), (d));                                                                                            \
    file_write((c)->asm_file, v);
#define asm_fwrite(c, a, d, f, ...)                                                                                    \
    write_spaces(c, (d));                                                                                              \
    file_write((c)->asm_file, arena_strformat((a), f, ##__VA_ARGS__));

void write_spaces(FX8664Compiler *c, int depth) {
    for (size_t i = 0; i < depth; i++)
        file_write(c->asm_file, "    ");
}

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
                             "public pypt_main\n\n"
                             "section '.text' executable align 16\n\n";

    file_write(c->asm_file, fasm_entry);
    return true;
}

void fasm_x86_64_compiler_append_expression(FX8664Compiler *c, ExprNode *expr) {
    if (expr->type == P_FUNC) {
        da_append(&c->funcs, expr);
        return;
    }

    da_append(&c->main, expr);
}

static CScope cscope_create() {
    CScope scope = {0};
    return scope;
}

static void cscope_destroy(CScope *s) {
    if (s->vars) {
        shfree(s->vars);
        s->vars = NULL;
    }
}

static CNode *node_create(Arena *a, ExprNode *expr) {
    CNode *node = (CNode *)arena_alloc(a, sizeof(*node));
    node->expr = expr;
    return node;
}

static CNode *_compile_expression(FX8664Compiler *c, CScope *scope, ExprNode *expr, Arena *a, int depth);

void fasm_x86_64_compiler_generate_assembly(FX8664Compiler *c) {
    for (size_t i = 0; i < c->funcs.count; i++) {
        ArenaNode saved = arena_save(&c->allocator);

        _compile_expression(c, &c->main_scope, c->funcs.items[i], &c->allocator, 0);

        arena_rewind(&c->allocator, saved);
    }

    asm_write(c, 0, "pypt_main:\n");
    asm_write(c, 1, "push rbp\n");
    asm_write(c, 1, "mov rbp, rsp\n\n");

    CNode *result = NULL;

    ArenaNode saved = arena_save(&c->allocator);
    for (size_t i = 0; i < c->main.count; i++) {
        ExprNode *current = c->main.items[i];
        if (current->type == P_RETURN) {
            CNode *child = _compile_expression(c, &c->main_scope, current->first, &c->allocator, 1);
            asm_fwrite(c, &c->allocator, 1, "mov rax, %s\n", child->reg);
            asm_fwrite(c, &c->allocator, 1, "jmp pypt_main_ret\n");
            result = node_create(&c->allocator, current);
            continue;
        }
        result = _compile_expression(c, &c->main_scope, current, &c->allocator, 1);
    }

    arena_rewind(&c->allocator, saved);

    if (result && result->expr->type != P_RETURN) {
        asm_fwrite(c, &c->allocator, 1, "mov rax, %s\n", result->reg);
        asm_fwrite(c, &c->allocator, 1, "jmp pypt_main_ret\n");
    }

    asm_write(c, 0, "pypt_main_ret:\n");
    asm_write(c, 1, "mov rsp, rbp\n");
    asm_write(c, 1, "pop rbp\n");
    asm_write(c, 1, "ret\n");

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

void cscope_assign(FX8664Compiler *c, CScope *s, const char *key, CNode *value, Arena *a, int depth) {
    CNode *var = shget(s->vars, key);

    if (var) {
        var->reg = value->reg;
        asm_fwrite(c, a, depth, "mov [rbp + 8*%d], %s\n", var->stack_index, value->reg);
    } else {
        value->stack_index = ++s->count;

        shput(s->vars, key, value);

        asm_fwrite(c, a, depth, "push %s\n", value->reg);
    }
}

CNode *_compile_expression(FX8664Compiler *c, CScope *scope, ExprNode *expr, Arena *a, int depth) {
    switch (expr->type) {
    case P_NUMBER: {
        // TODO: add suport to float
        CNode *node = node_create(a, expr);
        node->reg = "rcx";
        asm_fwrite(c, a, depth, "mov %s, %d\n", node->reg, (int)expr->number_value);
        return node;
    }
    case P_RETURN: {
        CNode *child = _compile_expression(c, scope, expr->first, a, depth);
        asm_fwrite(c, a, depth, "mov rax, %s\n", child->reg);
        asm_write(c, depth, "ret\n");

        CNode *result = node_create(a, expr);
        result->reg = "rax";
        return result;
    }
    case P_MINUS: {
        CNode *child = _compile_expression(c, scope, expr->first, a, depth);
        CNode *result = node_create(a, expr);
        result->reg = child->reg;
        asm_fwrite(c, a, depth, "neg %s\n", child->reg);
        return result;
    }
    case P_MULT: {
        CNode *first_child = _compile_expression(c, scope, expr->first, a, depth);
        asm_fwrite(c, a, depth, "push %s\n", first_child->reg);
        CNode *second_child = _compile_expression(c, scope, expr->second, a, depth);
        asm_fwrite(c, a, depth, "push %s\n", second_child->reg);
        asm_write(c, depth, "pop rcx\n");
        asm_write(c, depth, "pop rdx\n");
        asm_write(c, depth, "imul rcx, rdx\n");
        CNode *result = node_create(a, expr);
        result->reg = "rcx";
        return result;
    }
    case P_PLUS: {
        CNode *first_child = _compile_expression(c, scope, expr->first, a, depth);
        asm_fwrite(c, a, depth, "push %s\n", first_child->reg);
        CNode *second_child = _compile_expression(c, scope, expr->second, a, depth);
        asm_fwrite(c, a, depth, "push %s\n", second_child->reg);
        asm_write(c, depth, "pop rcx\n");
        asm_write(c, depth, "pop rdx\n");
        asm_write(c, depth, "add rcx, rdx\n");
        CNode *result = node_create(a, expr);
        result->reg = "rcx";
        return result;
    }
    case P_FUNC: {
        asm_fwrite(c, a, depth, "func_%s:\n", expr->string_value);
        asm_write(c, depth + 1, "push rbp\n");
        asm_write(c, depth + 1, "mov rbp, rsp\n\n");

        CScope func_scope = cscope_create();

        assert(expr->args.count <= 6); // TODO: ADD SUPORT TO MORE ARGS

        for (size_t i = 0; i < expr->args.count; i++) {
            ExprNode *arg = expr->args.items[i];
            assert(arg->type == P_IDENTIFIER);

            CNode *arg_node = node_create(a, arg);
            arg_node->reg = ARG_REGS[i];

            cscope_assign(c, &func_scope, arg->string_value, arg_node, a, depth + 1);
        }

        CNode *last = NULL;
        ExprNode *block_node = expr->first;
        for (size_t i = 0; i < block_node->count; i++) {
            ExprNode *current = block_node->items[i];
            if (current->type == P_RETURN) {
                CNode *child = _compile_expression(c, &func_scope, current->first, a, depth + 1);
                asm_fwrite(c, a, depth + 1, "mov rax, %s\n", child->reg);
                asm_fwrite(c, a, depth + 1, "jmp func_ret_%s\n", expr->string_value);
                last = node_create(a, current);
                continue;
            }
            last = _compile_expression(c, &func_scope, current, a, depth + 1);
        }

        if (last && last->expr->type != P_RETURN) {
            assert(last->reg);
            asm_fwrite(c, a, depth + 1, "mov rax, %s\n", last->reg);
        }

        asm_fwrite(c, a, depth, "func_ret_%s:\n", expr->string_value);
        asm_write(c, depth + 1, "mov rsp, rbp\n");
        asm_write(c, depth + 1, "pop rbp\n");
        asm_write(c, depth + 1, "ret\n");
        asm_fwrite(c, a, depth + 1, "; END func_%s\n\n", expr->string_value);
        break;
    }
    case P_FUN_CALL: {
        assert(expr->args.count <= 6); // TODO: ADD SUPORT TO MORE ARGS

        for (size_t i = 0; i < expr->args.count; i++) {
            CNode *arg = _compile_expression(c, scope, expr->args.items[i], a, depth);
            asm_fwrite(c, a, depth, "push %s\n", arg->reg);
        }

        for (int i = expr->args.count - 1; i >= 0; i--) {
            asm_fwrite(c, a, depth, "pop %s\n", ARG_REGS[i]);
        }

        asm_fwrite(c, a, depth, "call func_%s\n", expr->string_value);

        CNode *result = node_create(a, expr);
        result->reg = "rax";
        return result;
    }
    case P_IDENTIFIER: {
        CNode *var = shget(scope->vars, expr->string_value);
        if (!var) {
            fprintf(stderr, "Variable %s not found", expr->string_value);
            assert(var);
        }
        var->reg = "rax";
        asm_fwrite(c, a, depth, "mov %s, [rbp - 8*%d]\n", var->reg, var->stack_index);
        return var;
    }
    case P_ASSIGN: {
        CNode *value = _compile_expression(c, scope, expr->second, a, depth);

        assert(expr->first->type == P_IDENTIFIER);
        cscope_assign(c, scope, expr->first->string_value, value, a, depth);

        CNode *result = node_create(a, expr);
        result->reg = value->reg;
        return result;
    }
    case P_CHAR:
    case P_STRING:
    case P_BINOP:
    case P_DIV:
    case P_POW:
    case P_BLOCK:
    case P_IF:
    case P_WHILE:
    default:
        fprintf(stderr, "Type not implemented: %d\n", expr->type);
        assert(0 && "Type not implemented");
    }
}

#endif