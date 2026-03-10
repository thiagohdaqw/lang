#ifndef __ASM_COMPILER_H_INCLUDED__
#define __ASM_COMPILER_H_INCLUDED__

#include "parser.h"
#include "utils/arena.h"

#ifndef ASM_WORD_SIZE
#define ASM_WORD_SIZE 8
#endif

typedef enum {
    REG = 0,
    ADDR,
    STR,
} CLocationType;

typedef struct {
    CLocationType type;
    const char *identifier;
    size_t stack_index;
} CLocation;

typedef struct {
    ExprNode *expr;
    CLocation location;
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

typedef struct cscope_t {
    CVarMap *vars;
    int count;
    size_t if_count;
    size_t block_count;
    const char *identifier;

    struct cscope_t *parent;
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
void _generate_func_section(AsmCompiler *c);
void _generate_data_section(AsmCompiler *c);
void _generate_external_funcs_section(AsmCompiler *c);
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
#define asm_fwritel(c, a, d, f, ...) asm_fwrite((c), (a), (d), (f "\n"), ##__VA_ARGS__)

void write_spaces(AsmCompiler *c, int depth) {
    for (size_t i = 0; i < depth; i++)
        file_write(c->asm_file, "    ");
}

static CScope cscope_create(const char *identifier, CScope *parent) {
    CScope scope = {0};
    scope.identifier = identifier;
    scope.parent = parent;
    return scope;
}

AsmCompiler asm_compiler_create() {
    AsmCompiler compiler = {0};
    compiler.allocator = arena_create(4 * 1024);
    compiler.main_scope = cscope_create("pypt_main", NULL);
    return compiler;
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

static void load_address(AsmCompiler *c, Arena *a, int depth, const char *reg, CNode *node) {
    assert(node);
    assert(node->location.type == ADDR);
    asm_fwrite(c, a, depth, "mov %s, [%s]\n", reg, node->location.identifier);
}

static void fetch_node(AsmCompiler *c, Arena *a, int depth, const char *reg, CNode *node) {
    assert(node);
    assert(node->location.identifier);

    switch (node->location.type) {
    case REG:
        if (strcmp(node->location.identifier, reg) == 0) {
            break;
        }
        asm_fwritel(c, a, depth, "mov %s, %s", reg, node->location.identifier);
        break;
    case ADDR:
        asm_fwritel(c, a, depth, "mov %s, [%s]", reg, node->location.identifier);
        break;
    case STR:
        asm_fwritel(c, a, depth, "mov %s, %s", reg, node->location.identifier);
        break;
    default:
        assert(0 && "location type not implemented");
    }
}

static CNode *_compile_expression(AsmCompiler *c, CScope *scope, ExprNode *expr, Arena *a, int depth);

void asm_compiler_generate_assembly(AsmCompiler *c) {
    ArenaNode saved = arena_save(&c->allocator);

    _generate_func_section(c);

    asm_write(c, 0, "pypt_main:\n");
    asm_write(c, 1, "push rbp\n");
    asm_write(c, 1, "mov rbp, rsp\n\n");

    CNode *result = NULL;

    for (size_t i = 0; i < c->main.count; i++) {
        ExprNode *current = c->main.items[i];
        if (current->type == P_RETURN) {
            CNode *child = _compile_expression(c, &c->main_scope, current->first, &c->allocator, 1);
            fetch_node(c, &c->allocator, 1, "rax", child);
            if (i + 1 < c->main.count) {
                asm_fwrite(c, &c->allocator, 1, "jmp pypt_main_ret\n");
            }
            result = node_create(&c->allocator, current);
            result->location.identifier = "rax";
            continue;
        }
        result = _compile_expression(c, &c->main_scope, current, &c->allocator, 1);
    }

    if (result && result->expr->type != P_RETURN) {
        fetch_node(c, &c->allocator, 1, "rax", result);
    }

    asm_write(c, 0, "pypt_main_ret:\n");
    asm_write(c, 1, "mov rsp, rbp\n");
    asm_write(c, 1, "pop rbp\n");
    asm_write(c, 1, "ret\n\n");

    _generate_external_funcs_section(c);

    _generate_data_section(c);

    asm_write(c, 0, "\n");

    fclose(c->asm_file);
    arena_rewind(&c->allocator, saved);
}

void _generate_func_section(AsmCompiler *c) {
    for (size_t i = 0; i < c->funcs.count; i++) {
        // ArenaNode saved = arena_save(&c->allocator);

        _compile_expression(c, &c->main_scope, c->funcs.items[i], &c->allocator, 0);

        // arena_rewind(&c->allocator, saved);
    }
}

void _generate_data_section(AsmCompiler *c) {
    if (shlen(c->data) <= 0) return;

    asm_write(c, 0, "\n; Data section\n");
    asm_write(c, 0, "section '.data'\n");

    for (size_t i = 0; i < shlen(c->data); i++) {
        CNode *item = c->data[i].value;
        assert(item->location.identifier && item->location.type == STR);

        asm_fwrite(c, &c->allocator, 1, "%s db ", item->location.identifier);

        char *start = item->expr->string_value;
        for (char *buffer = start; *buffer != '\0'; buffer++) {
            if (*buffer == '\n') {
                asm_fwrite(c, &c->allocator, 0, "\"%.*s\",10,", buffer - start, start);
                start = buffer + 1;
            }
        }

        if (*start != '\0') {
            asm_fwrite(c, &c->allocator, 0, "\"%s\",0", start);
        } else {
            asm_write(c, 0, "0\n");
        }
    }
}

void _generate_external_funcs_section(AsmCompiler *c) {
    if (c->external_funcs.count <= 0) return;

    asm_write(c, 0, "; External funcs\n");
    for (size_t i = 0; i < c->external_funcs.count; i++) {
        asm_fwrite(c, &c->allocator, 0, "extrn %s\n", c->external_funcs.items[i]);
    }
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

static void _assign_var(AsmCompiler *c, CScope *s, Arena *a, int depth, CNode *identifier, CNode *value) {
    assert(identifier->expr && identifier->expr->type == P_IDENTIFIER);

    CScope *current = s;
    while (1) {
        if (shget(current->vars, identifier->expr->string_value)) {
            break;
        }
        current = current->parent;

        if (!current) {
            current = s;
            break;
        }
    }

    CNode *existing = shget(current->vars, identifier->expr->string_value);

    if (existing) {
        asm_fwrite(c, a, depth, "mov [%s], %s\n", existing->location.identifier, value->location.identifier);
        return;
    }

    identifier->location.type = ADDR;
    identifier->location.stack_index = ++s->count;
    identifier->location.identifier = arena_strformat(a, "rbp-%d", ASM_WORD_SIZE * identifier->location.stack_index);

    fetch_node(c, a, depth, "rax", value);
    asm_write(c, depth, "push rax\n");
    shput(current->vars, identifier->expr->string_value, identifier);
}

static void _create_array(AsmCompiler *c, CScope *s, Arena *a, int depth, CNode *identifier, int length) {
    assert(identifier->expr && identifier->expr->type == P_IDENTIFIER);

    CNode *existing = shget(s->vars, identifier->expr->string_value);

    if (existing) {
        assert(0 && "variable already defined");
        return;
    }

    asm_fwritel(c, a, depth, "sub rsp, %d", ASM_WORD_SIZE * length);
    asm_fwritel(c, a, depth, "mov rax, rsp");
    asm_fwritel(c, a, depth, "push rax");

    identifier->location.type = ADDR;
    identifier->location.stack_index = s->count + length + 1;
    identifier->location.identifier = arena_strformat(a, "rbp-%d", ASM_WORD_SIZE * identifier->location.stack_index);

    shput(s->vars, identifier->expr->string_value, identifier);
}

CNode *_compile_expression(AsmCompiler *c, CScope *scope, ExprNode *expr, Arena *a, int depth) {
    switch (expr->type) {
    case P_NUMBER: {
        // TODO: add support to float
        CNode *node = node_create(a, expr);
        node->location.identifier = "rcx";
        asm_fwrite(c, a, depth, "mov %s, %d\n", node->location.identifier, (int)expr->number_value);
        return node;
    }
    case P_RETURN: {
        CNode *child = _compile_expression(c, scope, expr->first, a, depth);
        asm_fwrite(c, a, depth, "mov rax, %s\n", child->location.identifier);
        asm_write(c, depth, "ret\n");

        CNode *result = node_create(a, expr);
        result->location.identifier = "rax";
        return result;
    }
    case P_MINUS: {
        CNode *child = _compile_expression(c, scope, expr->first, a, depth);
        CNode *result = node_create(a, expr);
        asm_fwrite(c, a, depth, "neg %s\n", child->location.identifier);
        switch (result->location.type) {
        case REG:
            result->location.identifier = child->location.identifier;
            break;
        case ADDR:
        default:
            assert(0 && "Location type not implemented");
        }
        return result;
    }
    case P_MULT:
    case P_PLUS: {
        CNode *first_child = _compile_expression(c, scope, expr->first, a, depth);

        fetch_node(c, a, depth, "rax", first_child);
        asm_fwritel(c, a, depth, "push rax");

        CNode *second_child = _compile_expression(c, scope, expr->second, a, depth);

        fetch_node(c, a, depth, "rcx", second_child);
        asm_fwrite(c, a, depth, "pop rdx\n");

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
        result->location.identifier = "rcx";
        return result;
    }
    case P_FUNC: {
        asm_fwrite(c, a, depth, "func_%s:\n", expr->string_value);
        asm_write(c, depth + 1, "push rbp\n");
        asm_write(c, depth + 1, "mov rbp, rsp\n\n");

        CScope func_scope = cscope_create(expr->string_value, scope);

        assert(expr->args.count <= 6); // TODO: ADD SUPORT TO MORE ARGS

        for (size_t i = 0; i < expr->args.count; i++) {
            ExprNode *arg = expr->args.items[i];
            assert(arg->type == P_IDENTIFIER);

            CNode *arg_node = node_create(a, arg);
            arg_node->location.identifier = ARG_REGS[i];

            _assign_var(c, &func_scope, a, depth + 1, arg_node, arg_node);
        }

        CNode *last = NULL;
        ExprNode *block_node = expr->first;
        for (size_t i = 0; i < block_node->count; i++) {
            ExprNode *current = block_node->items[i];
            if (current->type == P_RETURN) {
                CNode *child = _compile_expression(c, &func_scope, current->first, a, depth + 1);
                fetch_node(c, a, depth + 1, "rax", child);
                asm_fwrite(c, a, depth + 1, "jmp func_ret_%s\n", expr->string_value);
                last = node_create(a, current);
                continue;
            }
            last = _compile_expression(c, &func_scope, current, a, depth + 1);
        }

        if (last && last->expr->type != P_RETURN) {
            fetch_node(c, a, depth + 1, "rax", last);
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
        result->location.identifier = "rax";
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
        assert(expr->first->type == P_IDENTIFIER || expr->first->type == P_DEREF || expr->first->type == P_INDEX);

        CNode *result = node_create(a, expr);

        switch (expr->first->type) {
        case P_IDENTIFIER: {
            CNode *value = _compile_expression(c, scope, expr->second, a, depth);
            _assign_var(c, scope, a, depth, node_create(a, expr->first), value);
            result->location = value->location;
        } break;
        case P_DEREF: {
            CNode *id = _compile_expression(c, scope, expr->first->first, a, depth);
            fetch_node(c, a, depth, "rax", id);
            asm_fwritel(c, a, depth, "push rax");
            CNode *value = _compile_expression(c, scope, expr->second, a, depth);
            fetch_node(c, a, depth, "rdx", value);
            asm_fwritel(c, a, depth, "pop rax");
            asm_fwritel(c, a, depth, "mov [rax], rdx");
            result->location.type = REG;
            result->location.identifier = "rax";
        } break;
        case P_INDEX: {
            CNode *id = _compile_expression(c, scope, expr->first->first, a, depth);
            fetch_node(c, a, depth, "rax", id);
            asm_fwritel(c, a, depth, "push rax");
            CNode *index = _compile_expression(c, scope, expr->first->second, a, depth);
            fetch_node(c, a, depth, "rdx", index);
            asm_fwritel(c, a, depth, "imul rdx, %d", ASM_WORD_SIZE);
            asm_fwritel(c, a, depth, "pop rax");
            asm_fwritel(c, a, depth, "add rax, rdx");
            asm_fwritel(c, a, depth, "push rax");
            CNode *value = _compile_expression(c, scope, expr->second, a, depth);
            fetch_node(c, a, depth, "rdx", value);
            asm_fwritel(c, a, depth, "pop rax");
            asm_fwritel(c, a, depth, "mov [rax], rdx");
            result->location.type = REG;
            result->location.identifier = "rax";
        } break;
        default:
            assert(0 && "Assign type not implemented");
        }

        return result;
    }
    case P_STRING: {
        CNode *existing = shget(c->data, expr->string_value);

        if (existing) {
            CNode *value = node_create(a, expr);
            value->location.type = ADDR;
            value->location.identifier = existing->location.identifier;
            return value;
        } else {
            CNode *value = node_create(a, expr);
            value->location.identifier = arena_strformat(a, "dat_%d", shlen(c->data));
            value->location.type = STR;
            shput(c->data, expr->string_value, value);
            return value;
        }
    }
    case P_ARRAY: {
        assert(expr->first->type == P_IDENTIFIER);
        assert(expr->second->type == P_NUMBER);
        CNode *id = node_create(a, expr->first);
        _create_array(c, scope, a, depth, id, expr->second->number_value);
        return id;
    }
    case P_INDEX: {
        CNode *id = _compile_expression(c, scope, expr->first, a, depth);
        fetch_node(c, a, depth, "rax", id);
        asm_fwritel(c, a, depth, "push rax");
        CNode *index = _compile_expression(c, scope, expr->second, a, depth);
        fetch_node(c, a, depth, "rdx", index);
        asm_fwritel(c, a, depth, "imul rdx, %d", ASM_WORD_SIZE);
        asm_fwritel(c, a, depth, "pop rax");
        asm_fwritel(c, a, depth, "add rax, rdx");
        asm_fwritel(c, a, depth, "mov rax, [rax]");
        CNode *result = node_create(a, expr);
        result->location.identifier = "rax";
        return result;
    } break;
    case P_DEREF: {
        CNode *id = _compile_expression(c, scope, expr->first, a, depth);
        fetch_node(c, a, depth, "rax", id);
        asm_fwritel(c, a, depth, "mov rax, [rax]");
        CNode *result = node_create(a, expr);
        result->location.identifier = "rax";
        return result;
    }
    case P_IF: {
        int if_id = ++scope->if_count;

        CNode *result = node_create(a, expr);

        CNode *cond = _compile_expression(c, scope, expr->first, a, depth);
        fetch_node(c, a, depth, "rax", cond);
        asm_fwritel(c, a, depth, "cmp rax, 0");
        asm_fwritel(c, a, depth, "je .%s_if_%d_else", scope->identifier, if_id);

        if (expr->second) {
            _compile_expression(c, scope, expr->second, a, depth + 1);
            asm_fwritel(c, a, depth + 1, "jmp .%s_if_%d_end", scope->identifier, if_id);
        }
        asm_fwritel(c, a, depth, ".%s_if_%d_else:", scope->identifier, if_id);
        if (expr->third) {
            _compile_expression(c, scope, expr->third, a, depth + 1);
        }
        asm_fwritel(c, a, depth, ".%s_if_%d_end:", scope->identifier, if_id);
        result->location.identifier = "rax";
        return result;
    }
    case P_BLOCK: {
        int block_id = ++scope->block_count;
        CScope block_scope = cscope_create(arena_strformat(a, "%s_block_%d", scope->identifier, block_id), scope);

        CNode *last = NULL;

        for (size_t i = 0; i < expr->count; i++) {
            ExprNode *current = expr->items[i];
            last = _compile_expression(c, &block_scope, current, a, depth);
        }

        cscope_destroy(&block_scope);
        return last;
    }
    case P_CHAR:
    case P_DIV:
    case P_POW:
    case P_WHILE:
    default:
        fprintf(stderr, "Type not implemented: %d\n", expr->type);
        assert(0 && "Type not implemented");
    }
}

#endif