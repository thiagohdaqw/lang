#ifndef __ASM_COMPILER_H_INCLUDED__
#define __ASM_COMPILER_H_INCLUDED__

#include "../parser.h"
#include "../utils/arena.h"

#ifndef ASM_WORD_SIZE
#define ASM_WORD_SIZE "8"
#endif

typedef struct {
    ExprNode *expr;
    size_t stack_index;
    const char *reg;
    const char *address;
    const char *identifier;
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
    char **items;
    int capacity;
    int count;
} CExternalFuncs;

typedef struct {
    Arena allocator;

    const char *asm_file_path;
    FILE *asm_file;

    CExternalFuncs external_funcs;
    CBlock funcs;
    CBlock main;
    CScope main_scope;

    CVarMap *data;
} AsmCompiler;

AsmCompiler asm_compiler_create();
bool asm_compiler_init(AsmCompiler *c, const char *build_folder);
void asm_compiler_destroy(AsmCompiler *c);
void asm_compiler_append_expression(AsmCompiler *c, ExprNode *expr);
void asm_compiler_generate_assembly(AsmCompiler *c);
bool asm_compiler_compile(AsmCompiler *c, const char *object_output_path);

#endif // __ASM_COMPILER_H_INCLUDED__

#ifndef __ASM_COMPILER_IMP__
#define __ASM_COMPILER_IMP__

static const char *ARG_REGS[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

#define asm_write(c, d, v)                                                                                             \
    write_spaces((c), (d));                                                                                            \
    file_write((c)->asm_file, v);
#define asm_fwrite(c, a, d, f, ...)                                                                                    \
    write_spaces(c, (d));                                                                                              \
    file_write((c)->asm_file, arena_strformat((a), f, ##__VA_ARGS__));

void write_spaces(AsmCompiler *c, int depth) {
    for (size_t i = 0; i < depth; i++)
        file_write(c->asm_file, "    ");
}

AsmCompiler asm_compiler_create() {
    AsmCompiler compiler = {0};
    compiler.allocator = arena_create(4 * 1024);
    return compiler;
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

void asm_compiler_destroy(AsmCompiler *c) {
    arena_destroy(&c->allocator);
    cscope_destroy(&c->main_scope);
    da_destroy(&c->external_funcs);
    da_destroy(&c->main);
    shfree(c->data);
    c->data = NULL;
}

bool asm_compiler_init(AsmCompiler *c, const char *build_folder) {
    c->asm_file_path = arena_strjoin(&c->allocator, build_folder, "output.asm");
    c->asm_file = file_create(c->asm_file_path);

    const char *fasm_entry = "format ELF64\n"
                             "public pypt_main\n\n"
                             "section '.text' executable align 16\n\n";

    file_write(c->asm_file, fasm_entry);
    return true;
}

void asm_compiler_append_expression(AsmCompiler *c, ExprNode *expr) {
    if (expr->type == P_FUNC) {
        da_append(&c->funcs, expr);
        return;
    }

    da_append(&c->main, expr);
}

static CNode *node_create(Arena *a, ExprNode *expr) {
    CNode *node = (CNode *)arena_alloc(a, sizeof(*node));
    node->expr = expr;
    return node;
}

static CNode *_compile_expression(AsmCompiler *c, CScope *scope, ExprNode *expr, Arena *a, int depth);

void asm_compiler_generate_assembly(AsmCompiler *c) {
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

    if (result && result->expr->type != P_RETURN) {
        asm_fwrite(c, &c->allocator, 1, "mov rax, %s\n", result->reg);
        asm_fwrite(c, &c->allocator, 1, "jmp pypt_main_ret\n");
    }

    asm_write(c, 0, "pypt_main_ret:\n");
    asm_write(c, 1, "mov rsp, rbp\n");
    asm_write(c, 1, "pop rbp\n");
    asm_write(c, 1, "ret\n\n");

    asm_write(c, 0, "; External funcs\n");
    for (size_t i = 0; i < c->external_funcs.count; i++) {
        asm_fwrite(c, &c->allocator, 0, "extrn %s\n", c->external_funcs.items[i]);
    }

    asm_write(c, 0, "\n; Data section\n");
    asm_write(c, 0, "section '.data'\n");
    for (size_t i = 0; i < shlen(c->data); i++) {
        CNode *value = c->data[i].value;
        asm_fwrite(c, &c->allocator, 1, "%s db \"%s\",0\n", value->identifier, value->expr->scape_string_value);
    }

    asm_write(c, 0, "\n");

    fclose(c->asm_file);
    arena_rewind(&c->allocator, saved);
}

bool asm_compiler_compile(AsmCompiler *c, const char *object_output_path) {
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

static void fetch_node(AsmCompiler *c, Arena *a, int depth, const char *reg, CNode *node) {
    if (node->address) {
        asm_fwrite(c, a, depth, "mov %s, %s\n", reg, node->address);
    } else if (strcmp(node->reg, reg) != 0) {
        asm_fwrite(c, a, depth, "mov %s, %s\n", reg, node->reg);
    }
}

static void cscope_assign(AsmCompiler *c, CScope *s, CNode *identifier, CNode *value, Arena *a, int depth) {
    assert(identifier->expr && identifier->expr->type == P_IDENTIFIER);

    CNode *var = shget(s->vars, identifier->expr->string_value);

    if (var) {
        asm_fwrite(c, a, depth, "mov %s, %s\n", var->reg, value->reg);
    } else {
        identifier->stack_index = ++s->count;
        fetch_node(c, a, depth, "rax", value);
        asm_write(c, depth, "push rax\n");

        identifier->address = arena_strformat(a, "[rbp - " ASM_WORD_SIZE "*%d]", identifier->stack_index);
        identifier->reg = identifier->address;
        shput(s->vars, identifier->expr->string_value, identifier);
    }
}

CNode *_compile_expression(AsmCompiler *c, CScope *scope, ExprNode *expr, Arena *a, int depth) {
    switch (expr->type) {
    case P_NUMBER: {
        // TODO: add support to float
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
    case P_MULT:
    case P_PLUS: {
        CNode *first_child = _compile_expression(c, scope, expr->first, a, depth);
        if (!first_child->address) {
            asm_fwrite(c, a, depth, "push %s\n", first_child->reg);
        }
        CNode *second_child = _compile_expression(c, scope, expr->second, a, depth);

        fetch_node(c, a, depth, "rcx", second_child);

        if (!first_child->address) {
            asm_fwrite(c, a, depth, "pop rdx\n");
        } else {
            fetch_node(c, a, depth, "rdx", first_child);
        }

        switch (expr->type) {
        case P_MULT:
            asm_write(c, depth, "imul rcx, rdx\n");
            break;
        case P_PLUS:
            asm_write(c, depth, "add rcx, rdx\n");
            break;
        default:
            assert(0 && "op not implemented");
        }
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

            cscope_assign(c, &func_scope, arg_node, arg_node, a, depth + 1);
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

        cscope_destroy(&func_scope);
        break;
    }
    case P_FUNC_CALL: {
        bool func_exists = false;
        for (size_t i = 0; i < c->funcs.count; i++) {
            if (strcmp(expr->string_value, c->funcs.items[i]->string_value) == 0) {
                func_exists = true;
                break;
            }
        }

        if (!func_exists) {
            da_append(&c->external_funcs, expr->string_value);
        }

        assert(expr->args.count <= 6); // TODO: ADD SUPORT TO MORE ARGS
        for (size_t i = 0; i < expr->args.count; i++) {
            CNode *arg = _compile_expression(c, scope, expr->args.items[i], a, depth);
            fetch_node(c, a, depth, "rax", arg);
            asm_write(c, depth, "push rax\n");
        }

        for (int i = expr->args.count - 1; i >= 0; i--) {
            asm_fwrite(c, a, depth, "pop %s\n", ARG_REGS[i]);
        }

        if (func_exists) {
            asm_fwrite(c, a, depth, "call func_%s\n", expr->string_value);
        } else {
            asm_fwrite(c, a, depth, "call %s\n", expr->string_value);
        }

        CNode *result = node_create(a, expr);
        result->reg = "rax";
        return result;
    }
    case P_IDENTIFIER: {
        CNode *var = shget(scope->vars, expr->string_value);
        if (!var) {
            fprintf(stderr, "Variable '%s' not found\n", expr->string_value);
            assert(var);
        }
        return var;
    }
    case P_ASSIGN: {
        CNode *value = _compile_expression(c, scope, expr->second, a, depth);

        assert(expr->first->type == P_IDENTIFIER);
        cscope_assign(c, scope, node_create(a, expr->first), value, a, depth);

        CNode *result = node_create(a, expr);
        result->reg = value->reg;
        return result;
    }
    case P_STRING: {
        CNode *existing = shget(c->data, expr->string_value);
        CNode *value = node_create(a, expr);

        if (existing) {
            value->address = existing->address;
            value->reg = existing->address;
        } else {
            shput(c->data, expr->string_value, value);
            value->identifier = arena_strformat(a, "dat_%d", shlen(c->data));
            value->address = arena_strformat(a, "%s", value->identifier);
            value->reg = value->address;
        }

        return value;
    }
    case P_CHAR:
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