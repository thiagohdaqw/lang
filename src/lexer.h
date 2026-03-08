#ifndef __LEXER_H_INCLUDED__
#define __LEXER_H_INCLUDED__

#include <stdbool.h>
#include <stddef.h>

#define IDENTIFIER_MAX_LENGTH 1023
#define STRING_MAX_LENGTH 1023

typedef enum token_t {
    T_EOF,
    T_NEW_LINE,
    T_CHAR,
    T_STRING,
    T_LITERAL,
    T_IDENTIFIER,
    T_LONG,
    T_DOUBLE,
    T_PLUS,
    T_MINUS,
    T_MULT,
    T_DIV,
    T_POW,
    T_OPAREN,
    T_CPAREN,
    T_OBRACKET,
    T_CBRACKET,
    T_ASSIGN,
    T_EQUAL,
    T_LESS,
    T_LEQUAL,
    T_END,
    T_FUNC,
    T_IF,
    T_ELSE,
    T_WHILE,
    T_RETURN,
} TokenType;

typedef struct token {
    TokenType type;

    long long_value;
    double double_value;
    char literal_value;
    char char_value;

    char identifier_value[IDENTIFIER_MAX_LENGTH + 1];
    int identifier_size;

    char string_value[STRING_MAX_LENGTH + 1];
    int string_size;
} Token;

typedef struct reader {
    char *reader;
    long position;
    long size;
    long row;
    long col;
} Reader;

typedef struct lexer {
    const char *path;

    Token token;
    Reader reader;
} Lexer;

#define LEXER_ERROR_PRINT(l, fmt, ...)                                                                                 \
    fprintf(stderr, "[ERROR]: %s:%d:%d => " fmt, l->path, l->reader.row + 1, l->reader.col, ##__VA_ARGS__)

bool lexer_init(Lexer *lexer, const char *path);
void lexer_close(Lexer *lexer);
Reader lexer_save_reader(Lexer *lexer);
void lexer_rewind_reader(Lexer *lexer, Reader reader);
bool lexer_is_eof(Lexer *lexer);
bool lexer_next_token(Lexer *lexer);
void lexer_expect_token(Lexer *lexer, TokenType type);
void lexer_expect_literal(Lexer *lexer, char value);
Token lexer_peek_next_token(Lexer *lexer);

#endif // __LEXER_H_INCLUDED__

#ifndef __LEXER_H_IMP__
#define __LEXER_H_IMP__

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/file.h"

#define get_char(l) (l->reader.reader[l->reader.position])
#define is_eof(l)                                                                                                      \
    ((!l->reader.reader || l->reader.position >= l->reader.size || l->reader.position >= 0 && get_char(l) == 0))
#define is_identifier_char(c) (isalnum(c) || (c) == '_')

#define char_to_int(c) ((c) - '0')

bool lexer_is_eof(Lexer *lexer) { return is_eof(lexer); }

char lexer_peek_next_char(Lexer *lexer);

bool lexer_init(Lexer *lexer, const char *path) {
    memset(lexer, 0, sizeof(*lexer));

    lexer->path = path;
    lexer->reader.reader = file_read(path, &lexer->reader.size);
    lexer->reader.position = -1;

    if (!lexer->reader.reader || lexer->reader.size <= 0) {
        LEXER_ERROR_PRINT(lexer, "Failed to read %s: %s\n", path, strerror(errno));
        return false;
    }

    return true;
}

void lexer_close(Lexer *lexer) { free((void *)lexer->reader.reader); }

Reader lexer_save_reader(Lexer *lexer) { return lexer->reader; }
void lexer_rewind_reader(Lexer *lexer, Reader reader) { lexer->reader = reader; }

static char next_char(Lexer *lexer) {
    if (lexer->reader.position >= 0 && get_char(lexer) == '\n') {
        lexer->reader.row++;
        lexer->reader.col = 0;
    }
    lexer->reader.col++;
    lexer->reader.position++;
    return is_eof(lexer) ? 0 : get_char(lexer);
}

static bool skip_space(Lexer *lexer) {
    while (!is_eof(lexer) && isspace(get_char(lexer))) {
        if (get_char(lexer) == '\n') {
            return true;
        }
        next_char(lexer);
    }
    return !is_eof(lexer);
}

static bool parse_char(Lexer *lexer) {
    next_char(lexer);
    if (is_eof(lexer)) {
        LEXER_ERROR_PRINT(lexer, "Expected a character but got EOF\n");
        return false;
    }

    lexer->token.type = T_CHAR;
    lexer->token.char_value = get_char(lexer);

    next_char(lexer);
    if (is_eof(lexer)) {
        LEXER_ERROR_PRINT(lexer, "Expected a ' character but got EOF\n");
        return false;
    }
    if (get_char(lexer) != '\'') {
        LEXER_ERROR_PRINT(lexer, "Expected a ' character but got %c\n", get_char(lexer));
        return false;
    }
    return true;
}

static bool parse_string(Lexer *lexer) {
    lexer->token.type = T_STRING;
    lexer->token.string_size = 0;

    while (1) {
        char value = next_char(lexer);
        if (is_eof(lexer)) {
            LEXER_ERROR_PRINT(lexer, "Expected \" closing string but got EOF\n");
            return false;
        }
        if (value == '"') {
            break;
        }
        if (value == '\\') {
            if (lexer_peek_next_char(lexer) == 'n') {
                next_char(lexer);
                value = '\n';
            }
        }

        if (lexer->token.string_size + 1 > STRING_MAX_LENGTH) {
            LEXER_ERROR_PRINT(lexer, "String size cant be greater than %d characters\n", STRING_MAX_LENGTH - 1);
            return false;
        }
        lexer->token.string_value[lexer->token.string_size++] = value;
    }

    lexer->token.string_value[lexer->token.string_size] = '\0';
    return true;
}

static bool parse_literal(Lexer *lexer) {
    lexer->token.literal_value = get_char(lexer);

    switch (get_char(lexer)) {
    case '+':
        lexer->token.type = T_PLUS;
        break;
    case '-':
        lexer->token.type = T_MINUS;
        break;
    case '*':
        lexer->token.type = T_MULT;

        if (lexer_peek_next_char(lexer) == '*') {
            lexer_next_token(lexer);
            lexer->token.type = T_POW;
            break;
        }
        break;
    case '/':
        lexer->token.type = T_DIV;
        break;
    case '{':
        lexer->token.type = T_OBRACKET;
        break;
    case '}':
        lexer->token.type = T_CBRACKET;
        break;
    case '(':
        lexer->token.type = T_OPAREN;
        break;
    case ')':
        lexer->token.type = T_CPAREN;
        break;
    case '=': {
        lexer->token.type = T_ASSIGN;
        if (lexer_peek_next_char(lexer) == '=') {
            lexer->token.type = T_EQUAL;
            next_char(lexer);
        }
    } break;
    case '<': {
        lexer->token.type = T_LESS;
        if (lexer_peek_next_char(lexer) == '=') {
            lexer->token.type = T_LEQUAL;
            next_char(lexer);
        }
    } break;
    default:
        lexer->token.type = T_LITERAL;
        break;
    }

    return true;
}

static bool parse_identifier(Lexer *lexer) {
    lexer->token.identifier_size = 0;
    lexer->token.identifier_value[lexer->token.identifier_size++] = get_char(lexer);

    while (1) {
        Reader r = lexer_save_reader(lexer);

        next_char(lexer);
        if (is_eof(lexer) || !is_identifier_char(get_char(lexer))) {
            lexer_rewind_reader(lexer, r);
            break;
        }

        if (lexer->token.identifier_size + 1 > IDENTIFIER_MAX_LENGTH) {
            fprintf(stderr, "[LEXER]: %s:%d:%d => Unexpected identifier size too large\n", lexer->path,
                    lexer->reader.row, lexer->reader.col);
            return false;
        }
        lexer->token.identifier_value[lexer->token.identifier_size++] = get_char(lexer);
    }

    lexer->token.type = T_IDENTIFIER;
    lexer->token.identifier_value[lexer->token.identifier_size] = 0;

    char *id = lexer->token.identifier_value;
    if (strcmp(id, "fim") == 0) {
        lexer->token.type = T_END;
    } else if (strcmp(id, "func") == 0) {
        lexer->token.type = T_FUNC;
    } else if (strcmp(id, "se") == 0) {
        lexer->token.type = T_IF;
    } else if (strcmp(id, "senao") == 0) {
        lexer->token.type = T_ELSE;
    } else if (strcmp(id, "enquanto") == 0) {
        lexer->token.type = T_WHILE;
    } else if (strcmp(id, "retorne") == 0) {
        lexer->token.type = T_RETURN;
    }

    return true;
}

static bool parse_number(Lexer *lexer, int factor) {
    char *start = &lexer->reader.reader[lexer->reader.position];
    bool isDouble = false;

    while (1) {
        Reader r = lexer_save_reader(lexer);
        next_char(lexer);

        if (!is_eof(lexer) && !isDouble && get_char(lexer) == '.') {
            isDouble = true;
            continue;
        }

        if (is_eof(lexer) || !isdigit(get_char(lexer))) {
            lexer_rewind_reader(lexer, r);
            break;
        }
    }

    if (isDouble) {
        lexer->token.type = T_DOUBLE;
        lexer->token.double_value = factor * strtod(start, NULL);
        return true;
    }
    lexer->token.type = T_LONG;
    lexer->token.long_value = factor * strtol(start, NULL, 10);
    return true;
}

bool lexer_next_token(Lexer *lexer) {
    lexer->token.type = T_EOF;

    next_char(lexer);

    if (is_eof(lexer)) return false;

    if (!skip_space(lexer)) return false;

    if (get_char(lexer) == '\n') {
        lexer->token.type = T_NEW_LINE;
        return true;
    }

    if (isdigit(get_char(lexer))) {
        return parse_number(lexer, 1);
    }
    if (is_identifier_char(get_char(lexer))) {
        return parse_identifier(lexer);
    }
    if (get_char(lexer) == '\'') {
        return parse_char(lexer);
    }
    if (get_char(lexer) == '"') {
        return parse_string(lexer);
    }
    return parse_literal(lexer);
}

void lexer_expect_token(Lexer *lexer, TokenType type) {
    if (!lexer_next_token(lexer) || lexer->token.type != type) {
        LEXER_ERROR_PRINT(lexer, "Expected token %d but got token %d\n", type, lexer->token.type);
        exit(1);
    }
}

void lexer_expect_literal(Lexer *lexer, char value) {
    lexer_expect_token(lexer, T_LITERAL);
    if (lexer->token.literal_value != value) {
        LEXER_ERROR_PRINT(lexer, "Expected literal %c but got literal %c\n", value, lexer->token.literal_value);
        exit(1);
    }
}

char lexer_peek_next_char(Lexer *lexer) {
    Reader r = lexer_save_reader(lexer);
    char value = next_char(lexer);
    lexer_rewind_reader(lexer, r);
    return value;
}

Token lexer_peek_next_token(Lexer *lexer) {
    Reader r = lexer_save_reader(lexer);
    Token t = lexer->token;
    lexer_next_token(lexer);
    Token next_token = lexer->token;
    lexer->token = t;
    lexer_rewind_reader(lexer, r);
    return next_token;
}

#endif //__LEXER_H_IMP__