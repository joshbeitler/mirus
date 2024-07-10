#include <stdio.h>
#include "utils.h"
#include "math/calculator.h"

int main() {
    printf("Hello from the example C project!\n");

    int a = 10, b = 5;
    printf("Sum of %d and %d is: %d\n", a, b, add(a, b));
    printf("Product of %d and %d is: %d\n", a, b, multiply(a, b));

    char* message = "Hello, World!";
    printf("Original message: %s\n", message);
    printf("Reversed message: %s\n", reverse_string(message));

    return 0;
}
