// File: /c-vm-fac/c-vm-fac/src/vm.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdarg.h> // Include for va_list, va_start, va_end
#include "vm.h"

// Define a simple hash table for managing native libraries
typedef struct NativeLib {
    char name[256];
    void* handle;
    struct NativeLib* next;
} NativeLib;

static NativeLib* native_libs = NULL;
bool debug_enabled = false; // Define the debug flag as a global variable

void debug_print(const char* format, ...) { // Remove static to make it accessible globally
    if (!debug_enabled) return; // Only print if debug mode is enabled
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

static void add_native_library(const char* key, void* handle) {
    NativeLib* lib = malloc(sizeof(NativeLib));
    strncpy(lib->name, key, sizeof(lib->name) - 1);
    lib->name[sizeof(lib->name) - 1] = '\0';
    lib->handle = handle;
    lib->next = native_libs;
    native_libs = lib;
}

static void print_native_libs() {
    if (!debug_enabled) return; // Only print if debug mode is enabled
    printf("[vm] Current native libs:\n");
    NativeLib* current = native_libs;
    while (current) {
        printf("  - [%s] => %p\n", current->name, current->handle);
        current = current->next;
    }
}

static void* get_native_library(const char* key) {
    NativeLib* current = native_libs;
    while (current) {
        if (strcmp(current->name, key) == 0) {
            return current->handle;
        }
        current = current->next;
    }
    return NULL;
}

static void free_native_libraries() {
    NativeLib* current = native_libs;
    while (current) {
        NativeLib* next = current->next;
        if (current->handle) {
            dlclose(current->handle);
        }
        free(current);
        current = next;
    }
    native_libs = NULL;
}

void vm_init(VM *vm, const unsigned char *code, size_t code_size) {
    vm->code = code;
    vm->code_size = code_size;
    vm->pc = 0;
    vm->reg = 0;
    vm->int_reg = 0;
    vm->str_buf[0] = '\0';
}

static void write_int(int value) {
    char buf[32];
    int i = 30, neg = 0;
    buf[31] = '\0';
    if (value == 0) {
        buf[i--] = '0';
    } else {
        if (value < 0) {
            neg = 1;
            value = -value;
        }
        while (value > 0 && i >= 0) {
            buf[i--] = '0' + (value % 10);
            value /= 10;
        }
        if (neg && i >= 0) buf[i--] = '-';
    }
    write(1, &buf[i+1], 30-i);
}

static void execute_covil_file(const char* path, const char* argument) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        write(2, "Cannot open .covil file\n", 24);
        exit(1);
    }

    // Read and verify magic number
    unsigned char mbuf[4];
    fread(mbuf, 1, 4, f);
    uint32_t magic = ((uint32_t)mbuf[0] << 24) |
                     ((uint32_t)mbuf[1] << 16) |
                     ((uint32_t)mbuf[2] << 8)  |
                     ((uint32_t)mbuf[3]);
    if (magic != MAGIC) {
        write(2, "Invalid magic number in .covil file\n", 36);
        fclose(f);
        exit(1);
    }

    // Read the rest of the bytecode
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f) - 4;
    fseek(f, 4, SEEK_SET);
    unsigned char* code = malloc(size);
    fread(code, 1, size, f);
    fclose(f);

    // Initialize and execute the VM for the .covil file
    VM covil_vm;
    vm_init(&covil_vm, code, size);

    // Pass the argument to the .covil VM
    strncpy(covil_vm.str_buf, argument, sizeof(covil_vm.str_buf) - 1);
    covil_vm.str_buf[sizeof(covil_vm.str_buf) - 1] = '\0';

    vm_execute(&covil_vm, true); // Pass true for is_covil

    free(code);
}

static void call_printnlio(const char* s, size_t len) { // ojos tristes
    (void)len; // Mark len as unused to suppress warning
    const char* path = "/workspaces/Covione/CoviSys/standardlibs/Covio.covil";

    // Execute the Covio.covil file with the provided argument
    execute_covil_file(path, s);
}

static void load_native_library(const char* path) {
    void* handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        write(2, "Failed to load native library\n", 30);
        write(2, dlerror(), strlen(dlerror()));
        write(2, "\n", 1);
        exit(1);
    }
    add_native_library(path, handle);
}

static void call_native_function(const char* libName, const char* funcName) {
    void* handle = get_native_library(libName);
    if (!handle) {
        exit(1);
    }
    void (*func)() = (void (*)())dlsym(handle, funcName);
    if (!func) {
        write(2, "Failed to load native function\n", 31);
        write(2, dlerror(), strlen(dlerror()));
        write(2, "\n", 1);
        exit(1);
    }
    func();
}

static void* cnilink_handle = NULL;

static void load_cnilink() {
    if (!cnilink_handle) {
        cnilink_handle = dlopen("/workspaces/Covione/CoviSys/NativeLib/cnilink.so", RTLD_LAZY);
        if (!cnilink_handle) {
            write(2, "Failed to load cnilink.so\n", 26);
            write(2, dlerror(), strlen(dlerror()));
            write(2, "\n", 1);
            exit(1);
        }
    }
}

static void call_cnilink_load_library(const char* path) {
    load_cnilink();
    void* (*cnilink_load_library)(const char*) = dlsym(cnilink_handle, "cnilink_load_library");
    if (!cnilink_load_library) {
        write(2, "Failed to locate cnilink_load_library\n", 38);
        write(2, dlerror(), strlen(dlerror()));
        write(2, "\n", 1);
        exit(1);
    }
    void* handle = cnilink_load_library(path);
    // Extract basename from full path, remove ".so" suffix.
    const char *slash = strrchr(path, '/');
    const char *libname = slash ? slash + 1 : path;
    char key[256];
    strncpy(key, libname, sizeof(key) - 1);
    key[sizeof(key) - 1] = '\0';
    char* dot = strstr(key, ".so");
    if (dot) {
        *dot = '\0';
    }
    add_native_library(key, handle);
}

void vm_execute(VM *vm, bool is_covil) {
    if (!is_covil) { // Skip main method validation for .covil files
        if (vm->pc < vm->code_size && vm->code[vm->pc] == OP_MAIN) {
            vm->pc++;
            uint8_t objLen = vm->code[vm->pc++];
            char obj[256];
            memcpy(obj, &vm->code[vm->pc], objLen);
            obj[objLen] = '\0';
            vm->pc += objLen;

            uint8_t methLen = vm->code[vm->pc++];
            char meth[256];
            memcpy(meth, &vm->code[vm->pc], methLen);
            meth[methLen] = '\0';
            vm->pc += methLen;

            uint8_t argLen = vm->code[vm->pc++];
            if (vm->pc + argLen > vm->code_size) {
                write(2, "Invalid argument length\n", 25);
                exit(1);
            }
            vm->pc += argLen;

            if (strcmp(meth, "main") != 0) {
                write(2, "No main method defined\n", 24);
                exit(1);
            }
        } else {
            write(2, "No main method header found\n", 29);
            exit(1);
        }
    }

    while (vm->pc < vm->code_size) {
        unsigned char opcode = vm->code[vm->pc++];
        switch (opcode) {
            case OP_LOAD_BYTE:
                if (vm->pc < vm->code_size) {
                    vm->reg = vm->code[vm->pc++];
                }
                break;
            case OP_PRINT_BYTE:
                write(1, &vm->reg, 1);
                break;
            case OP_LOAD_INT:
                if (vm->pc + 3 < vm->code_size) {
                    vm->int_reg = ((uint32_t)vm->code[vm->pc] << 24) |
                                 ((uint32_t)vm->code[vm->pc+1] << 16) |
                                 ((uint32_t)vm->code[vm->pc+2] << 8) |
                                  (uint32_t)vm->code[vm->pc+3];
                    vm->pc += 4;
                }
                break;
            case OP_PRINT_INT:
                write_int(vm->int_reg);
                break;
            case OP_LOAD_STR: {
                if (vm->pc < vm->code_size) {
                    size_t len = vm->code[vm->pc++];
                    if (vm->pc + len <= vm->code_size && len < sizeof(vm->str_buf)) {
                        memcpy(vm->str_buf, &vm->code[vm->pc], len);
                        vm->str_buf[len] = '\0';
                        vm->pc += len;
                    }
                }
                break;
            }
            case OP_PRINT_STR:
                write(1, vm->str_buf, strlen(vm->str_buf));
                break;
            case OP_PRINTNLIO: {
                // Read object name
                uint8_t objLen = vm->code[vm->pc++];
                vm->pc += objLen; // Skip object name

                // Read group name
                uint8_t groupLen = vm->code[vm->pc++];
                vm->pc += groupLen; // Skip group name

                // Read method name
                uint8_t methLen = vm->code[vm->pc++];
                vm->pc += methLen; // Skip method name

                // Read argument (string to print)
                uint8_t argLen = vm->code[vm->pc++];
                if (vm->pc + argLen - 1 < vm->code_size && argLen < sizeof(vm->str_buf)) {
                    memcpy(vm->str_buf, &vm->code[vm->pc], argLen);
                    vm->str_buf[argLen] = '\0';
                    vm->pc += argLen;

                    // Output the argument directly
                    printf("%s\n", vm->str_buf);
                } else {
                    write(2, "Error: Invalid argument length for OP_PRINTNLIO\n", 48);
                    exit(1);
                }
                break;
            }
            case OP_LOAD_NATIVE: {
                uint8_t pathLen = vm->code[vm->pc++];
                char path[256];
                memcpy(path, &vm->code[vm->pc], pathLen);
                path[pathLen] = '\0';
                vm->pc += pathLen;
                load_native_library(path);
                break;
            }
            case OP_CALL_NATIVE: {
                uint8_t libNameLen = vm->code[vm->pc++];
                char libName[256];
                memcpy(libName, &vm->code[vm->pc], libNameLen);
                libName[libNameLen] = '\0';
                vm->pc += libNameLen;

                uint8_t funcNameLen = vm->code[vm->pc++];
                char funcName[256];
                memcpy(funcName, &vm->code[vm->pc], funcNameLen);
                funcName[funcNameLen] = '\0';
                vm->pc += funcNameLen;

                call_native_function(libName, funcName);
                break;
            }
            case OP_NATIVELINKLOCATE: {
                uint8_t pathLen = vm->code[vm->pc++];
                char path[256];
                memcpy(path, &vm->code[vm->pc], pathLen);
                path[pathLen] = '\0';
                vm->pc += pathLen;
                call_cnilink_load_library(path);
                break;
            }
            case OP_LOAD_NATIVE_LIB: {
                uint8_t pathLen = vm->code[vm->pc++];
                char path[256];
                memcpy(path, &vm->code[vm->pc], pathLen);
                path[pathLen] = '\0';
                vm->pc += pathLen;
                load_native_library(path);
                break;
            }
            case OP_HALT:
                free_native_libraries(); // Free native libraries on halt
                return;
            default:
                exit(1); // Suppress unknown opcode logs
        }
    }
}