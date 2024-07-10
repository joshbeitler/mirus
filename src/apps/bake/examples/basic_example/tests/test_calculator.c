#include <stdio.h>
#include <assert.h>
#include "math/calculator.h"

void test_add() {
    assert(add(2, 3) == 5);
    assert(add(-1, 1) == 0);
    printf("test_add passed\n");
}

void test_multiply() {
    assert(multiply(2, 3) == 6);
    assert(multiply(-2, 4) == -8);
    printf("test_multiply passed\n");
}

int main() {
    test_add();
    test_multiply();
    return 0;
}
