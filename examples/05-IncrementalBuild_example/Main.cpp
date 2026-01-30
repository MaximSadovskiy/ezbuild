#include <stdio.h>
#include "Test.h"

void second();

int main(int argc, char**argv) {
    second();

    if (argc > 1) printf("Got argument %s\n", argv[1]);
}
