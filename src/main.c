#include<stdio.h>

#include "lexer.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "[MAIN]: No file path specified\n");
        fprintf(stderr, "[MAIN]: Usage: %s <file_path>\n", argv[0]);
        return 1;
    }
    
    
    Lexer lexer = {0};
    if (!lexer_init(&lexer, argv[1])) {
        return 1;
    }

    while (lexer_next_token(&lexer)) {
        printf("%s:%d:%d =>\t", lexer.path, lexer.reader.row + 1, lexer.reader.col);
        switch (lexer.token.type)
        {
        case T_STRING:
            printf("STRING(%s)\n", lexer.token.string_value);
            break;
        case T_CHAR:
            printf("CHAR(%c)\n", lexer.token.char_value);
            break;
        case T_LITERAL:
            printf("LITERAL(%c)\n", lexer.token.literal_value);
            break;
        case T_INT:
            printf("INT(%d)\n", lexer.token.long_value);
            break;
        case T_IDENTIFIER:
            printf("IDENTIFIER(%s)\n", lexer.token.identifier_value);
            break;
        case T_DOUBLE:
            printf("DOUBLE(%lf)\n", lexer.token.double_value);
            break;
        default:
            printf("NOT_IMPLEMENTED(%d)\n", lexer.token.type);
            break;
        }
    }
    printf("EOF\n");
    return 0;
}