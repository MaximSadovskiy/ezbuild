#include <stdio.h>

int main() {
#ifdef DEBUG_CONFIG
    printf("Hello debug\n");
#else
    printf("Hello release\n");
#endif
}  