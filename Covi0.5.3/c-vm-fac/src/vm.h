// filepath: /c-vm-fac/c-vm-fac/src/vm.h
#ifndef VM_H
#define VM_H

#include <stddef.h>
#include <stdint.h>

#define MAGIC 0xFAACBEED

typedef struct {
    const unsigned char *code;
    size_t code_size;
    size_t pc; // program counter
    unsigned char reg; // temporary register
    int int_reg; // register for integer values
    char str_buf[256]; // buffer for string values
} VM;

// Opcodes
#define OP_LOAD_BYTE  0x01
#define OP_PRINT_BYTE 0x02
#define OP_HALT       0xFF
#define OP_LOAD_INT   0x10
#define OP_PRINT_INT  0x11
#define OP_LOAD_STR   0x20
#define OP_PRINT_STR  0x21
#define OP_PRINTNLIO  0x30
#define OP_MAIN       0x40
#define OP_LOAD_NATIVE 0x50 // Opcode for loading a native library
#define OP_CALL_NATIVE 0x51 // Opcode for calling a function in a native library
#define OP_NATIVELINKLOCATE 0x60 // Opcode for locating a native library
#define OP_LOAD_NATIVE_LIB 0x25 // New opcode for loading a native library binary

// Function prototypes
void vm_init(VM *vm, const unsigned char *code, size_t code_size);
void vm_execute(VM *vm, bool is_covil); // Updated prototype
void vm_free(VM *vm);

#endif // VM_H