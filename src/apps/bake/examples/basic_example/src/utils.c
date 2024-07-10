#include "utils.h"
#include <string.h>
#include <stdlib.h>

char* reverse_string(const char* str) {
    int length = strlen(str);
    char* reversed = malloc(length + 1);

    for (int i = 0; i < length; i++) {
        reversed[i] = str[length - 1 - i];
    }
    reversed[length] = '\0';

    return reversed;
}
