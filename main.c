#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "./sha256.h"


#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define BLUE "\033[0;34m"
#define YELLOW "\033[0;33m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define COLOR_RESET "\033[0m"

#ifndef __GNUC__
#define __attribute__(X)
#define UNUSED(X) (void)(X)
#else
#define UNUSED(X)
#endif

#define die(void) die_(__FILE__, __LINE__)
void die_(const char *file_name, unsigned int line) __attribute__((noreturn));
char *dir_path(const char *base_name, const char *dir_name);
void print_file_recursively(const char *target);
void print_hash(char *target_path);
char hex_digit(unsigned int digit);
void hash_string(BYTE hash[SHA256_BLOCK_SIZE], char output[SHA256_BLOCK_SIZE*2 + 1]);

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    UNUSED(argc);
    UNUSED(argv);

    //print_file_recursively("/home/abhinesh/test");
    print_file_recursively(".");

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
            printf("%sfile%s: %s\n", GREEN, COLOR_RESET, target_path);
            print_hash(target_path);
            printf("-------------------------------------\n");
        }
        free(target_path);
        ent = readdir(dir);
    }

    if (errno)
        die();

    closedir(dir);
}

void print_hash(char *target_path) {
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

    if (ferror(fd))
        die();

    BYTE hash[SHA256_BLOCK_SIZE];
    sha256_final(&ctx, hash);

    char output[32*2 + 1];
    hash_string(hash, output);
    printf("%shash%s: %s\n", RED, COLOR_RESET, output);
}

char hex_digit(unsigned int digit) {
    digit = digit % 0x10;
    if (digit <= 9) return digit + '0';
    if (10 <= digit && digit <= 15) return digit - 10 + 'a';
    assert(0 && "unreachable");
}

void hash_string(BYTE hash[SHA256_BLOCK_SIZE], char output[SHA256_BLOCK_SIZE*2 + 1]) {
    for(size_t i = 0; i < SHA256_BLOCK_SIZE; ++i) {
        output[i*2 + 0] = hex_digit(hash[i] / 0x10);
        output[i*2 + 1] = hex_digit(hash[i]);
    }
    output[SHA256_BLOCK_SIZE*2] = '\0';
}
