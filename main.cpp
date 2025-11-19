#include <iostream>
#include <dlfcn.h>
#include <cassert>
#include "misc/dr4_ifc.hpp"

const char PLAGIN_PATH[] = "external/libAIPlugin.so";
typedef dr4::DR4Backend* (*DR4BackendFunction)();


int main() {
    void *backendLib = dlopen(PLAGIN_PATH, RTLD_LAZY);
    DR4BackendFunction func = (DR4BackendFunction) dlsym(backendLib, dr4::DR4BackendFunctionName); assert(func);
    dr4::DR4Backend *backend = func(); assert(backend);
    dr4::Window *window = backend->CreateWindow(); assert(window);


    std::cout << "Hello, World!\n";
}
