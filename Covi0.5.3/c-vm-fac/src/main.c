// File: /c-vm-fac/c-vm-fac/src/main.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> // For write
#include <string.h>
#include <stdbool.h> // For bool, true, false
#include "vm.h"

// Declare the debug_print function
void debug_print(const char* format, ...);

#define MAGIC 0xFAACBEED

extern bool debug_enabled; // Declare the debug flag

uint32_t read_big_endian_uint32(const unsigned char* bytes) {
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8)  |
           ((uint32_t)bytes[3]);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        write(2, "Usage: ./vm [-debug] file.fac\n", 30);
        return 1;
    }

    int file_arg_index = 1;
    if (strcmp(argv[1], "-debug") == 0) {
        debug_enabled = true; // Enable debug mode
        file_arg_index = 2;
    }

    if (file_arg_index >= argc) {
        write(2, "Usage: ./vm [-debug] file.fac\n", 30);
        return 1;
    }

    // Check if the file ends with ".covil"
    const char *file_path = argv[file_arg_index];
    bool is_covil = false;
    const char *ext = strrchr(file_path, '.');
    if (ext && strcmp(ext, ".covil") == 0) {
        is_covil = true;
    }

    FILE *f = fopen(file_path, "rb");
    if (!f) {
        write(2, "Cannot open file\n", 17);
        return 1;
    }

    // Read magic number as 4 separate bytes
    unsigned char mbuf[4];
    fread(mbuf, 1, 4, f);
    uint32_t magic = read_big_endian_uint32(mbuf); // Use helper function to handle endianness
    if (magic != MAGIC) {
        write(2, "Invalid magic number\n", 21);
        fclose(f);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f) - 4;
    fseek(f, 4, SEEK_SET);
    unsigned char *code = malloc(size);
    fread(code, 1, size, f);
    fclose(f);

    VM vm;
    vm_init(&vm, code, size);
    vm_execute(&vm, is_covil); // Pass the is_covil flag to the VM

    free(code);
    return 0;
}