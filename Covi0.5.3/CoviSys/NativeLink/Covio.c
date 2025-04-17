#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

void printnl(const char* s) {
    size_t len = 0;
    while (s[len]) len++;

    unsigned char code[] = {
        0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,             // mov rax, 1
        0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,             // mov rdi, 1
        0x48, 0xbe,                                           // mov rsi, <s>
        0, 0, 0, 0, 0, 0, 0, 0,                                // (8 bytes for pointer, starting at index 16)
        0x48, 0xba,                                           // mov rdx, <len>
        0, 0, 0, 0, 0, 0, 0, 0,                                // (8 bytes for len, starting at index 26)
        0x0f, 0x05,                                           // syscall
        0xc3                                                  // ret
    };

    /*
      Expected layout (index):
      0-6:   mov rax,1
      7-13:  mov rdi,1
      14-15: mov rsi, opcode (0x48, 0xbe)
      16-23: immediate (pointer s)
      24-25: mov rdx, opcode (0x48, 0xba)
      26-33: immediate (len)
      34-35: syscall (0x0f, 0x05)
      36:    ret (0xc3)
    */

    // Copy the pointer s into the immediate field for rsi (starting at index 16)
    memcpy(&code[16], &s, 8);

    // Copy the length into the immediate field for rdx (starting at index 26)
    memcpy(&code[26], &len, 8);

    void* exec = mmap(NULL, 4096,
                      PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1, 0);

    memcpy(exec, code, sizeof(code));
    ((void(*)())exec)();

    munmap(exec, 4096);

    char nl = '\n';
    write(1, &nl, 1);
}
