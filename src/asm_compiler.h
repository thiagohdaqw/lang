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
    int stack_count;
    size_t if_count;
    size_t block_count;
    size_t while_count;
    const char *identifier;
    const char *ret;

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
    CBlock export_funcs;
    CBlock main;
    CScope main_scope;

    CVarMap *data;
} AsmCompiler;

AsmCompiler asm_compiler_create();
bool asm_compiler_init(AsmCompiler *c, const char *build_folder, bool output_object);
void asm_compiler_destroy(AsmCompiler *c);
void asm_compiler_append_expression(AsmCompiler *c, ExprNode *expr);
void asm_compiler_generate_assembly(AsmCompiler *c, bool output_main);
bool asm_compiler_compile(AsmCompiler *c, const char *object_output_path);

#endif // __ASM_COMPILER_H_INCLUDED__

#ifndef __ASM_COMPILER_IMP__
#define __ASM_COMPILER_IMP__

#define WORD64 0
#define WORD32 1
#define WORD16 2
#define WORD8 3

static const char *ARG_REGS[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static const char *RAX_ARG[] = {"rax", "eax", "ax", "al"};
static const char *RCX_ARG[] = {"rcx", "ecx", "cx", "cl"};
static const char *RDX_ARG[] = {"rdx", "edx", "dx", "dl"};

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

static CScope cscope_create(const char *identifier, CScope *parent, const char *ret) {
    CScope scope = {0};
    scope.identifier = identifier;
    scope.parent = parent;
    scope.ret = ret;

    if (parent) {
        scope.stack_count = parent->stack_count;
    }
    return scope;
}

AsmCompiler asm_compiler_create() {
    AsmCompiler compiler = {0};
    compiler.allocator = arena_create(4 * 1024);
    compiler.main_scope = cscope_create("pypt_main", NULL, "pypt_main_ret");
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

bool asm_compiler_init(AsmCompiler *c, const char *build_folder, bool output_object) {
    c->asm_file_path = arena_strjoin(&c->allocator, build_folder, "output.asm");
    c->asm_file = file_create(c->asm_file_path);

    file_write(c->asm_file, "format ELF64\n");
    if (!output_object) {
        file_write(c->asm_file, "public pypt_main\n");
    }
    file_write(c->asm_file, "section '.text' executable align 16\n\n");
    return true;
}

void asm_compiler_append_expression(AsmCompiler *c, ExprNode *expr) {
    if (expr->type == P_FUNC) {
        if (expr->exportable) {
            da_append(&c->export_funcs, expr);
        }
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

static void _append_external_function(AsmCompiler *c, ExprNode *expr) {
    bool existing = false;
    for (size_t i = 0; i < c->external_funcs.count; i++) {
        if (strcmp(expr->string_value, c->external_funcs.items[i]) == 0) {
            existing = true;
            break;
        }
    }

    if (existing) return;

    da_append(&c->external_funcs, expr->string_value);
}

static int wordsize_index(int v) {
    switch (v) {
    case 0:
        return WORD64;
    case 1:
        return WORD8;
    case 2:
        return WORD16;
    case 4:
        return WORD32;
    case 8:
        return WORD64;
    default:
        fprintf(stderr, "Invalid var size %d\n", v);
        exit(1);
    }
}

static int wordsize_value(int v) {
    switch (v) {
    case WORD64:
        return 8;
    case WORD32:
        return 4;
    case WORD16:
        return 8;
    case WORD8:
        return 1;
    default:
        fprintf(stderr, "Invalid var size %d\n", v);
        exit(1);
    }
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
        asm_fwritel(c, a, depth, "lea %s, [%s]", reg, node->location.identifier);
        break;
    default:
        assert(0 && "location type not implemented");
    }
}

static CNode *_compile_expression(AsmCompiler *c, CScope *scope, Arena *a, int depth, ExprNode *expr,
                                  const char *out_reg[], int word_size);
static void _generate_export_funcs_section(AsmCompiler *c);
static void _generate_func_section(AsmCompiler *c);
static void _generate_external_funcs_section(AsmCompiler *c);
static void _generate_data_section(AsmCompiler *c);

void _generate_main_section(AsmCompiler *c) {
    asm_write(c, 0, "pypt_main:\n");
    asm_write(c, 1, "push rbp\n");
    asm_write(c, 1, "mov rbp, rsp\n\n");

    CNode *result = NULL;

    for (size_t i = 0; i < c->main.count; i++) {
        ExprNode *current = c->main.items[i];
        if (current->type == P_RETURN) {
            CNode *child = _compile_expression(c, &c->main_scope, &c->allocator, 1, current->first, RAX_ARG, WORD64);
            fetch_node(c, &c->allocator, 1, "rcx", child);
            asm_fwritel(c, &c->allocator, 1, "xor rax, rax");
            asm_fwritel(c, &c->allocator, 1, "mov rax, rcx");
            if (i + 1 < c->main.count) {
                asm_fwrite(c, &c->allocator, 1, "jmp pypt_main_ret\n");
            }
            result = node_create(&c->allocator, current);
            result->location.identifier = "rax";
            continue;
        }
        result = _compile_expression(c, &c->main_scope, &c->allocator, 1, current, RAX_ARG, WORD64);
    }

    if (!result || result->expr->type != P_RETURN) {
        asm_fwritel(c, &c->allocator, 1, "mov rax, 0");
    }

    asm_write(c, 0, "pypt_main_ret:\n");
    asm_write(c, 1, "mov rsp, rbp\n");
    asm_write(c, 1, "pop rbp\n");
    asm_write(c, 1, "ret\n\n");
}

void asm_compiler_generate_assembly(AsmCompiler *c, bool output_main) {
    ArenaNode saved = arena_save(&c->allocator);

    _generate_export_funcs_section(c);
    _generate_func_section(c);

    if (output_main) {
        _generate_main_section(c);
    }

    _generate_external_funcs_section(c);

    _generate_data_section(c);

    asm_write(c, 0, "\n");

    fclose(c->asm_file);
    arena_rewind(&c->allocator, saved);
}

void _generate_export_funcs_section(AsmCompiler *c) {
    asm_fwritel(c, &c->allocator, 0, "\n; Public Functions");
    for (size_t i = 0; i < c->export_funcs.count; i++) {
        asm_fwritel(c, &c->allocator, 0, "public %s", c->export_funcs.items[i]->string_value);
    }

    asm_fwritel(c, &c->allocator, 0, "; End Public Functions\n");
}

void _generate_func_section(AsmCompiler *c) {
    for (size_t i = 0; i < c->funcs.count; i++) {
        // ArenaNode saved = arena_save(&c->allocator);

        _compile_expression(c, &c->main_scope, &c->allocator, 0, c->funcs.items[i], RAX_ARG, WORD64);

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
            asm_fwritel(c, &c->allocator, 0, "\"%s\",0", start);
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
        if (value->location.type == ADDR) {
            asm_fwrite(c, a, depth, "mov rax, [%s]\n", value->location.identifier);
            asm_fwrite(c, a, depth, "mov [%s], rax\n", existing->location.identifier);
        } else {
            asm_fwrite(c, a, depth, "mov [%s], %s\n", existing->location.identifier, value->location.identifier);
        }
        return;
    }

    fetch_node(c, a, depth, "rax", value);
    asm_write(c, depth, "push rax\n");

    shput(current->vars, identifier->expr->string_value, identifier);

    identifier->location.type = ADDR;
    identifier->location.stack_index = ++s->stack_count;
    identifier->location.identifier = arena_strformat(a, "rbp-%d", ASM_WORD_SIZE * identifier->location.stack_index);
}

static void _create_array(AsmCompiler *c, CScope *s, Arena *a, int depth, CNode *identifier, int length) {
    assert(identifier->expr && identifier->expr->type == P_IDENTIFIER);

    CNode *existing = shget(s->vars, identifier->expr->string_value);

    if (existing) {
        assert(0 && "variable already defined");
        return;
    }

    int word_size = wordsize_index(identifier->expr->size);

    int block_size = length = wordsize_value(word_size);
    int block_allocation = (ASM_WORD_SIZE + block_size) / ASM_WORD_SIZE;

    asm_fwritel(c, a, depth, "sub rsp, %d", block_allocation * ASM_WORD_SIZE);
    asm_fwritel(c, a, depth, "mov %s, rsp", RAX_ARG[WORD64]);
    asm_fwritel(c, a, depth, "push %s", RAX_ARG[WORD64]);

    s->stack_count = s->stack_count + block_allocation + 1;
    identifier->location.stack_index = s->stack_count;
    identifier->location.type = ADDR;

    identifier->location.identifier = arena_strformat(a, "rbp-%d", ASM_WORD_SIZE * identifier->location.stack_index);

    shput(s->vars, identifier->expr->string_value, identifier);
}

void _compile_func(AsmCompiler *c, CScope *s, Arena *a, int depth, ExprNode *expr) {
    char *prefix = expr->exportable ? "" : "func_";

    asm_fwrite(c, a, depth, "%s%s:\n", prefix, expr->string_value);
    asm_write(c, depth + 1, "push rbp\n");
    asm_write(c, depth + 1, "mov rbp, rsp\n\n");

    CScope func_scope =
        cscope_create(expr->string_value, NULL, arena_strformat(a, "%s%s_ret", prefix, expr->string_value));

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
            CNode *child = _compile_expression(c, &func_scope, a, depth + 1, current->first, RAX_ARG, WORD64);
            fetch_node(c, a, depth + 1, "rcx", child);
            asm_fwritel(c, &c->allocator, 1, "xor rax, rax");
            asm_fwritel(c, &c->allocator, 1, "mov rax, rcx");

            asm_fwrite(c, a, depth + 1, "jmp %s\n", func_scope.ret);
            last = node_create(a, current);
            continue;
        }
        last = _compile_expression(c, &func_scope, a, depth + 1, current, RAX_ARG, WORD64);
    }

    if (last && last->expr->type != P_RETURN && last->location.identifier) {
        fetch_node(c, a, depth + 1, "rcx", last);
        asm_fwritel(c, &c->allocator, 1, "xor rax, rax");
        asm_fwritel(c, &c->allocator, 1, "mov rax, rcx");
    }

    asm_fwrite(c, a, depth, "%s:\n", func_scope.ret);
    asm_write(c, depth + 1, "mov rsp, rbp\n");
    asm_write(c, depth + 1, "pop rbp\n");
    asm_write(c, depth + 1, "ret\n");
    asm_fwrite(c, a, depth + 1, "; END %s%s\n\n", prefix, expr->string_value);

    cscope_destroy(&func_scope);
}

CNode *_compile_expression(AsmCompiler *c, CScope *scope, Arena *a, int depth, ExprNode *expr, const char *out_reg[],
                           int word_size) {
    CNode *result = node_create(a, expr);
    result->location.identifier = out_reg[word_size];

    switch (expr->type) {
    case P_NUMBER: {
        // TODO: add support to float
        asm_fwrite(c, a, depth, "mov %s, %d\n", out_reg[WORD64], (int)expr->number_value);
        return result;
    }
    case P_RETURN: {
        CNode *child = _compile_expression(c, scope, a, depth, expr->first, RAX_ARG, word_size);
        result->location.identifier = RAX_ARG[word_size];
        fetch_node(c, a, depth, "rcx", child);
        asm_fwritel(c, &c->allocator, 1, "xor rax, rax");
        asm_fwritel(c, &c->allocator, 1, "mov rax, rcx");
        asm_write(c, depth, "ret\n");
        return result;
    }
    case P_MINUS: {
        CNode *child = _compile_expression(c, scope, a, depth, expr->first, out_reg, word_size);
        fetch_node(c, a, depth, result->location.identifier, child);
        asm_fwrite(c, a, depth, "neg %s\n", result->location.identifier);
        switch (result->location.type) {
        case REG:
            fetch_node(c, a, depth, result->location.identifier, child);
            break;
        case ADDR:
        default:
            assert(0 && "Location type not implemented");
        }
        return result;
    }
    case P_SHIFTL:
    case P_SHIFTR:
    case P_EQUAL:
    case P_NEQUAL:
    case P_GREATER:
    case P_GEQUAL:
    case P_LESS:
    case P_LEQUAL:
    case P_AND:
    case P_OR:
    case P_MULT:
    case P_PLUS: {
        CNode *first_child = _compile_expression(c, scope, a, depth, expr->first, RAX_ARG, word_size);
        fetch_node(c, a, depth, RAX_ARG[word_size], first_child);
        asm_fwritel(c, a, depth, "push %s", RAX_ARG[word_size]);

        CNode *second_child = _compile_expression(c, scope, a, depth, expr->second, RCX_ARG, word_size);
        fetch_node(c, a, depth, RCX_ARG[word_size], second_child);
        asm_fwrite(c, a, depth, "pop %s\n", RDX_ARG[word_size]);

        result->location.identifier = RDX_ARG[word_size];

        switch (expr->type) {
        case P_MULT:
            asm_fwritel(c, a, depth, "imul %s, %s", RDX_ARG[word_size], RCX_ARG[word_size]);
            break;
        case P_PLUS:
            asm_fwritel(c, a, depth, "add %s, %s", RDX_ARG[word_size], RCX_ARG[word_size]);
            break;
        case P_EQUAL:
            asm_write(c, depth, "xor rax, rax\n");
            asm_fwritel(c, a, depth, "cmp %s, %s", RDX_ARG[word_size], RCX_ARG[word_size]);
            asm_write(c, depth, "sete ah\n");
            result->location.identifier = "rax";
            break;
        case P_NEQUAL:
            asm_write(c, depth, "xor rax, rax\n");
            asm_fwritel(c, a, depth, "cmp %s, %s", RDX_ARG[word_size], RCX_ARG[word_size]);
            asm_write(c, depth, "setne ah\n");
            result->location.identifier = "rax";
            break;
        case P_GREATER:
            asm_write(c, depth, "xor rax, rax\n");
            asm_fwritel(c, a, depth, "cmp %s, %s", RDX_ARG[word_size], RCX_ARG[word_size]);
            asm_write(c, depth, "setg ah\n");
            result->location.identifier = "rax";
            break;
        case P_GEQUAL:
            asm_write(c, depth, "xor rax, rax\n");
            asm_fwritel(c, a, depth, "cmp %s, %s", RDX_ARG[word_size], RCX_ARG[word_size]);
            asm_write(c, depth, "setge ah\n");
            result->location.identifier = "rax";
            break;
        case P_LESS:
            asm_write(c, depth, "xor rax, rax\n");
            asm_fwritel(c, a, depth, "cmp %s, %s", RDX_ARG[word_size], RCX_ARG[word_size]);
            asm_write(c, depth, "setl ah\n");
            result->location.identifier = "rax";
            break;
        case P_LEQUAL:
            asm_write(c, depth, "xor rax, rax\n");
            asm_fwritel(c, a, depth, "cmp %s, %s", RDX_ARG[word_size], RCX_ARG[word_size]);
            asm_write(c, depth, "setle ah\n");
            result->location.identifier = "rax";
            break;
        case P_AND:
            asm_fwritel(c, a, depth, "and %s, %s", RDX_ARG[word_size], RCX_ARG[word_size]);
            break;
        case P_OR:
            asm_fwritel(c, a, depth, "or %s, %s", RDX_ARG[word_size], RCX_ARG[word_size]);
            break;
        case P_SHIFTL:
            asm_fwritel(c, a, depth, "shl %s, %s", RDX_ARG[word_size], RCX_ARG[WORD8]);
            break;
        case P_SHIFTR:
            asm_fwritel(c, a, depth, "shr %s, %s", RDX_ARG[word_size], RCX_ARG[WORD8]);
            break;
        default:
            assert(0 && "op not implemented");
        }
        return result;
    }
    case P_FUNC:
        _compile_func(c, scope, a, depth, expr);
        return NULL;
    case P_FUNC_CALL: {
        ExprNode *existing_func = NULL;
        for (size_t i = 0; i < c->funcs.count; i++) {
            if (strcmp(expr->string_value, c->funcs.items[i]->string_value) == 0) {
                existing_func = c->funcs.items[i];
                break;
            }
        }

        if (!existing_func) {
            _append_external_function(c, expr);
        }

        assert(expr->args.count <= 6); // TODO: ADD SUPORT TO MORE ARGS
        for (size_t i = 0; i < expr->args.count; i++) {
            CNode *arg = _compile_expression(c, scope, a, depth, expr->args.items[i], RAX_ARG, word_size);
            fetch_node(c, a, depth, RAX_ARG[word_size], arg);
            asm_fwritel(c, a, depth, "push %s\n", RAX_ARG[word_size]);
        }

        for (int i = expr->args.count - 1; i >= 0; i--) {
            asm_fwrite(c, a, depth, "pop %s\n", ARG_REGS[i]);
        }

        if (existing_func) {
            char *prefix = existing_func->exportable ? "" : "func_";
            asm_fwrite(c, a, depth, "call %s%s\n", prefix, expr->string_value);
        } else {
            // Align stack to 16bytes
            asm_fwritel(c, a, depth, "mov rax, rsp");
            asm_fwritel(c, a, depth, "sub rsp, 16");
            asm_fwritel(c, a, depth, "and rsp, -16");
            asm_fwritel(c, a, depth, "push 0");
            asm_fwritel(c, a, depth, "push rax");
            asm_fwrite(c, a, depth, "call %s\n", expr->string_value);
            asm_fwritel(c, a, depth, "pop rsp");
        }

        result->location.identifier = RAX_ARG[word_size];
        return result;
    }
    case P_IDENTIFIER: {
        CNode *var = shget(scope->vars, expr->string_value);
        CScope *s = scope;
        while (!var) {
            s = s->parent;
            if (!s) break;
            var = shget(s->vars, expr->string_value);
        }
        if (!var) {
            fprintf(stderr, "Variable '%s' not found\n", expr->string_value);
            assert(var);
        }
        return var;
    }
    case P_ASSIGN: {
        assert(expr->first->type == P_IDENTIFIER || expr->first->type == P_DEREF || expr->first->type == P_INDEX);

        switch (expr->first->type) {
        case P_IDENTIFIER: {
            int word_size = wordsize_index(expr->first->size);
            CNode *value = _compile_expression(c, scope, a, depth, expr->second, out_reg, word_size);
            _assign_var(c, scope, a, depth, node_create(a, expr->first), value);
            result->location = value->location;
        } break;
        case P_DEREF: {
            CNode *id = _compile_expression(c, scope, a, depth, expr->first->first, RAX_ARG, word_size);
            fetch_node(c, a, depth, RAX_ARG[word_size], id);
            asm_fwritel(c, a, depth, "push %s", RAX_ARG[word_size]);
            CNode *value = _compile_expression(c, scope, a, depth, expr->second, RDX_ARG, word_size);
            fetch_node(c, a, depth, RDX_ARG[word_size], value);
            asm_fwritel(c, a, depth, "pop %s", RAX_ARG[word_size]);
            asm_fwritel(c, a, depth, "mov [%s], %s", RAX_ARG[WORD64], RDX_ARG[word_size]);
            result->location.type = REG;
            result->location.identifier = RAX_ARG[word_size];
        } break;
        case P_INDEX: {
            assert(expr->first->first->type == P_IDENTIFIER);
            word_size = WORD64;
            CNode *id = _compile_expression(c, scope, a, depth, expr->first->first, RAX_ARG, word_size);
            fetch_node(c, a, depth, RAX_ARG[word_size], id);
            asm_fwritel(c, a, depth, "push %s", RAX_ARG[word_size]);
            CNode *index = _compile_expression(c, scope, a, depth, expr->first->second, RDX_ARG, word_size);
            fetch_node(c, a, depth, RDX_ARG[word_size], index);
            asm_fwritel(c, a, depth, "imul %s, %d", RDX_ARG[word_size], wordsize_value(wordsize_index(id->expr->size)));
            asm_fwritel(c, a, depth, "pop %s", RAX_ARG[word_size]);
            asm_fwritel(c, a, depth, "add %s, %s", RAX_ARG[word_size], RDX_ARG[word_size]);
            asm_fwritel(c, a, depth, "push %s", RAX_ARG[word_size]);
            CNode *value = _compile_expression(c, scope, a, depth, expr->second, RDX_ARG, word_size);
            fetch_node(c, a, depth, RDX_ARG[word_size], value);
            asm_fwritel(c, a, depth, "pop %s", RAX_ARG[word_size]);

            word_size = wordsize_index(id->expr->size);
            asm_fwritel(c, a, depth, "mov [%s], %s", RAX_ARG[WORD64], RDX_ARG[word_size]);
            result->location.type = REG;
            result->location.identifier = RAX_ARG[WORD64];
        } break;
        default:
            assert(0 && "Assign type not implemented");
        }

        return result;
    }
    case P_STRING: {
        CNode *existing = shget(c->data, expr->string_value);
        if (existing) {
            result->location = existing->location;
            return result;
        } else {
            result->location.identifier = arena_strformat(a, "dat_%d", shlen(c->data));
            result->location.type = STR;
            shput(c->data, expr->string_value, result);
            return result;
        }
    }
    case P_ARRAY: {
        assert(expr->first->type == P_IDENTIFIER);
        assert(expr->second->type == P_NUMBER);
        result->expr = expr->first;
        _create_array(c, scope, a, depth, result, expr->second->number_value);
        return result;
    }
    case P_INDEX: {
        assert(expr->first->type == P_IDENTIFIER);
        word_size = WORD64;
        CNode *id = _compile_expression(c, scope, a, depth, expr->first, RAX_ARG, word_size);
        fetch_node(c, a, depth, RAX_ARG[word_size], id);
        asm_fwritel(c, a, depth, "push rax");
        CNode *index = _compile_expression(c, scope, a, depth, expr->second, RDX_ARG, word_size);
        fetch_node(c, a, depth, "rdx", index);
        asm_fwritel(c, a, depth, "imul rdx, %d", ASM_WORD_SIZE);
        asm_fwritel(c, a, depth, "pop rax");
        asm_fwritel(c, a, depth, "add rax, rdx");
        asm_fwritel(c, a, depth, "xor rcx, rcx");
        asm_fwritel(c, a, depth, "mov rcx, [rax]");
        word_size = wordsize_index(expr->first->size);
        asm_fwritel(c, a, depth, "mov %s, %s", out_reg[word_size], RCX_ARG[word_size]);
        return result;
    } break;
    case P_DEREF: {
        CNode *id = _compile_expression(c, scope, a, depth, expr->first, RAX_ARG, WORD64);
        fetch_node(c, a, depth, RAX_ARG[WORD64], id);
        asm_fwritel(c, a, depth, "mov rdx, [%s]", id->location.identifier);
        asm_fwritel(c, a, depth, "mov %s, %s", result->location.identifier, RDX_ARG[word_size]);
        return result;
    }
    case P_IF: {
        int if_id = ++scope->if_count;

        CNode *cond = _compile_expression(c, scope, a, depth, expr->first, out_reg, word_size);
        fetch_node(c, a, depth, result->location.identifier, cond);
        asm_fwritel(c, a, depth, "cmp %s, 0", result->location.identifier);
        asm_fwritel(c, a, depth, "je .if_%d_else", if_id);

        if (expr->second) {
            _compile_expression(c, scope, a, depth + 1, expr->second, RAX_ARG, word_size);
            asm_fwritel(c, a, depth + 1, "jmp .if_%d_end", if_id);
        }
        asm_fwritel(c, a, depth, ".if_%d_else:", if_id);
        if (expr->third) {
            _compile_expression(c, scope, a, depth + 1, expr->third, RAX_ARG, word_size);
        }
        asm_fwritel(c, a, depth, ".if_%d_end:", if_id);
        return result;
    }
    case P_BLOCK: {
        int block_id = ++scope->block_count;
        CScope block_scope =
            cscope_create(arena_strformat(a, "%s_block_%d", scope->identifier, block_id), scope, scope->ret);

        CNode *last = NULL;

        for (size_t i = 0; i < expr->count; i++) {
            ExprNode *current = expr->items[i];
            if (current->type == P_RETURN) {
                CNode *child = _compile_expression(c, &block_scope, a, depth + 1, current->first, RAX_ARG, word_size);
                fetch_node(c, a, depth + 1, "rcx", child);
                asm_fwritel(c, &c->allocator, 1, "xor rax, rax");
                asm_fwritel(c, &c->allocator, 1, "mov rax, rcx");
                asm_fwritel(c, a, depth + 1, "jmp %s", block_scope.ret);
                last = node_create(a, current);
                continue;
            }
            last = _compile_expression(c, &block_scope, a, depth, current, RAX_ARG, word_size);
        }

        cscope_destroy(&block_scope);
        return last;
    }
    case P_WHILE: {
        int while_id = ++scope->while_count;

        asm_fwritel(c, a, depth, ".%s_while_%d_cond:", scope->identifier, while_id);
        CNode *cond = _compile_expression(c, scope, a, depth + 1, expr->first, RAX_ARG, word_size);
        fetch_node(c, a, depth + 1, RAX_ARG[word_size], cond);
        asm_fwritel(c, a, depth + 1, "cmp %s, 0", RAX_ARG[word_size]);
        asm_fwritel(c, a, depth + 1, "je .%s_while_%d_end", scope->identifier, while_id);
        asm_fwritel(c, a, depth, ".%s_while_%d_body:", scope->identifier, while_id);
        _compile_expression(c, scope, a, depth + 1, expr->second, RAX_ARG, word_size);
        asm_fwritel(c, a, depth, "jmp .%s_while_%d_cond", scope->identifier, while_id);
        asm_fwritel(c, a, depth, ".%s_while_%d_end:", scope->identifier, while_id);
        return result;
    }
    case P_CHAR: {
        asm_fwritel(c, a, depth, "mov %s, %d", result->location.identifier, expr->char_value);
        return result;
    }
    case P_DIV:
    case P_POW:
    default:
        fprintf(stderr, "Type not implemented: %d\n", expr->type);
        assert(0 && "Type not implemented");
    }
}

#endif