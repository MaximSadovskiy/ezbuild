#include "inner/Test2.h"

#include <stdio.h>

void second() {
    #ifdef TEST_DEFINE
        printf("Hello from second!\n");
    #else
        printf("Bye from second!\n");
    #endif
}