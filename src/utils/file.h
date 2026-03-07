#ifndef __FILE_H_INCLUDED__
#define __FILE_H_INCLUDED__

char* file_read(const char* filename, long* size);
FILE* file_create(const char* filename);
size_t file_write(FILE* f, const char* value);
void file_close(FILE* file);
bool folder_create(const char* folder, int mode);

#endif

#ifndef __FILE_H_IMP__
#define __FILE_H_IMP__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

char* file_read(const char* filename, long* size) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = (char*)malloc(*size + 1);
    fread(buffer, 1, *size, f);
    buffer[*size] = '\0';

    fclose(f);
    return buffer;
}

FILE* file_create(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Failed to create file '%s': %s\n", filename, strerror(errno));
        assert(0 && "Failed to create file");
    }
    return f;
}

void file_close(FILE* file) {
    if (file) {
        fclose(file);
    }
}

size_t file_write(FILE* f, const char* value) {
    return fwrite(value, 1, strlen(value), f);
}

bool folder_create(const char* folder, int mode) {
    if (mkdir(folder, mode) != 0) {
        if (errno == EEXIST) return true;
        fprintf(stderr, "Failed to create build folder '%s': %s\n", folder, strerror(errno));
        return false;
    }
    return true;
}

#endif