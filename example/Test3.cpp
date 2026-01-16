#include <stdio.h>
#include "Test.h"

int main() {
    #if defined(__clang__)
        auto compiler = "clang";
    #elif defined(_MSC_VER)
        auto compiler = "msvc";
    #else
        auto compiler = "wtf";
    #endif
    printf("Hello from %s, async!\n", compiler);
}
