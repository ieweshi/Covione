#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

// Function to load a shared library and return its handle
void* cnilink_load_library(const char* path) {
    printf("[cnilink] Loading library: %s\n", path);
    void* handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "[cnilink] Failed to load library: %s\n", dlerror());
        exit(1);
    }
    return handle;
}

// Function to call a function in a loaded shared library
void cnilink_call_function(void* handle, const char* func_name) {
    printf("[cnilink] Calling function: %s\n", func_name);
    void (*func)() = (void (*)())dlsym(handle, func_name);
    if (!func) {
        fprintf(stderr, "[cnilink] Failed to locate function '%s': %s\n", func_name, dlerror());
        exit(1);
    }
    func();
}

// Function to close a shared library
void cnilink_close_library(void* handle) {
    if (handle) {
        printf("[cnilink] Closing library\n");
        dlclose(handle);
    }
}
