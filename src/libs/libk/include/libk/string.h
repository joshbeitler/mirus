#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(const char *str);

bool checkStringEndsWith(const char *str, const char *end);
void reverse(const char *str);
char *itoa(int num, char *str, int base);
