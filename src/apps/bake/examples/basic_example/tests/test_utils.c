#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "utils.h"

void test_reverse_string() {
    char* original = "Hello";
    char* reversed = reverse_string(original);
    assert(strcmp(reversed, "olleH") == 0);
    printf("test_reverse_string passed\n");
    free(reversed);
}

int main() {
    test_reverse_string();
    return 0;
}
