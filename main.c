#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "sha256.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define BLUE "\033[0;34m"
#define YELLOW "\033[0;33m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define COLOR_RESET "\033[0m"

typedef struct {
    BYTE bytes[SHA256_BLOCK_SIZE];
} HASH;

typedef struct {
    HASH key;
    char **paths;
} CELL;

CELL *table = NULL;

#define die(void) die_(__FILE__, __LINE__)
void die_(const char *file_name, unsigned int line);
char *dir_path(const char *base_name, const char *dir_name);
void print_file_recursively(const char *target);
void get_hash(char *target_path, HASH *Hash);
void path_free(CELL *table);
char hex_digit(unsigned int digit);
void hash_string(HASH *hash, char output[SHA256_BLOCK_SIZE*2 + 1]);
void print(CELL *table);

#define SEP_INT 47
int main(int argc, char *argv[])
{
    (void) argc;
    
    if (argc != 2) {
        fprintf(stderr, "NO Path\n");
        exit(EXIT_FAILURE);
    }

    if ((int)argv[1][strlen(argv[1]) - 1] == SEP_INT ) {
        char path[strlen(argv[1])]; 
        size_t i;
        for(i = 0; i < strlen(argv[1]) - 1; ++i)
            path[i] = argv[1][i];
        path[i] = '\0';
        print_file_recursively(path);
    }
    else
        print_file_recursively(argv[1]);

    print(table);
    path_free(table);

    return EXIT_SUCCESS;
}

void die_(const char *file_name, unsigned int line) {
    fprintf(stdout,
            "%s:%u: %sERROR%s: %s%s%s\n",
            file_name, 
            line,
            RED,
            COLOR_RESET,
            GREEN,
            strerror(errno),
            COLOR_RESET); 
    exit(EXIT_FAILURE); 
}

#define SEP "/"
#define SEP_LEN 1
char *dir_path(const char *base_name, const char *dir_name) {
    size_t base_len = strlen(base_name);
    size_t dir_len = strlen(dir_name);

    char *path = malloc(base_len + dir_len + SEP_LEN + 1);
    assert(path != NULL);
    char *end = path;
    memcpy(end, base_name, base_len);
    end += base_len;
    memcpy(end, SEP, SEP_LEN);
    end += SEP_LEN;
    memcpy(end, dir_name, dir_len);
    end += dir_len;
    *end = '\0';

    return path;
}

void print_file_recursively(const char *target) {
    DIR *dir = opendir(target);
    if (!dir)   
        die();

    errno = 0;
    struct dirent *ent = readdir(dir);
    while(ent != NULL) {
        char *target_path = dir_path(target, ent->d_name);
        if (ent->d_type == DT_DIR) {
            if ((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..") != 0)) {
                print_file_recursively(target_path);
            }
        }
        else {
            HASH hash;
            get_hash(target_path, &hash);
            
            ptrdiff_t index = hmgeti(table , hash);
            if (index < 0 ) {
                CELL cell;
                cell.key = hash;
                cell.paths = NULL;
                arrput(cell.paths, target_path);
                hmputs(table, cell);
            }
            else
                arrput(table[index].paths, target_path);
        }

        ent = readdir(dir);
    }

    if (errno)
        die();

    closedir(dir);
}

void path_free(CELL *table) {
    for(ptrdiff_t i = 0; i < hmlen(table); ++i) {
        for(ptrdiff_t j = 0; j < arrlen(table[i].paths); ++j)
                free(table[i].paths[j]);
    }
}

void get_hash(char *target_path, HASH *Hash) {
    SHA256_CTX ctx;
    sha256_init(&ctx);

    FILE *fd = fopen(target_path, "rb");
    if (fd == NULL)
        die();

    BYTE buf[1024];
    size_t buffer = fread(buf, 1, sizeof(buf), fd);
    while(buffer > 0) {
        sha256_update(&ctx, buf, buffer);
        buffer = fread(buf, 1, sizeof(buf), fd);
    }

    if (ferror(fd)) {
        die();
    }

    fclose(fd);

    sha256_final(&ctx, Hash->bytes);
}

char hex_digit(unsigned int digit) {
    digit = digit % 0x10;
    if (digit <= 9) return digit + '0';
    if (10 <= digit && digit <= 15) return digit - 10 + 'a';
    assert(0 && "unreachable");
}

void hash_string(HASH *hash, char output[SHA256_BLOCK_SIZE*2 + 1]) {
    for(size_t i = 0; i < SHA256_BLOCK_SIZE; ++i) {
        output[i*2 + 0] = hex_digit(hash->bytes[i] / 0x10);
        output[i*2 + 1] = hex_digit(hash->bytes[i]);
    }
    output[SHA256_BLOCK_SIZE*2] = '\0';
}
void print(CELL *table) {
    for(ptrdiff_t i = 0 ; i < hmlen(table); ++i) {
        if (arrlen(table[i].paths) > 1) {
            char output[65];
            hash_string(&table[i].key, output);
            printf("%shash%s: %s\n", GREEN, COLOR_RESET, output);

            for(ptrdiff_t j = 0; j < arrlen(table[i].paths); ++j) {
                printf("\t%s\n", table[i].paths[j]);
            }
        }
    }
}
