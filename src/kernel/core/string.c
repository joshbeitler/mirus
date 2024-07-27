#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kernel/string.h>

void *memcpy(void *dest, const void *src, size_t n) {
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  for (size_t i = 0; i < n; i++) {
    pdest[i] = psrc[i];
  }

  return dest;
}

void *memset(void *s, int c, size_t n) {
  uint8_t *p = (uint8_t *)s;

  for (size_t i = 0; i < n; i++) {
    p[i] = (uint8_t)c;
  }

  return s;
}

void *memmove(void *dest, const void *src, size_t n) {
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  if (src > dest) {
    for (size_t i = 0; i < n; i++) {
      pdest[i] = psrc[i];
    }
  } else if (src < dest) {
    for (size_t i = n; i > 0; i--) {
      pdest[i-1] = psrc[i-1];
    }
  }

  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t *p1 = (const uint8_t *)s1;
  const uint8_t *p2 = (const uint8_t *)s2;

  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] < p2[i] ? -1 : 1;
    }
  }

  return 0;
}

size_t strlen(const char *str) {
  size_t ret = 0;

  while (*str++) {
    ret++;
  }

  return ret;
}

bool checkStringEndsWith(const char *str, const char *end) {
  const char *_str = str;
  const char *_end = end;

  while (*str != 0) {
    str++;
  }

  str--;

  while (*end != 0) {
    end++;
  }

  end--;

  while (true) {
    if (*str != *end) {
      return false;
    }

    str--;
    end--;

    if (end == _end || (str == _str && end == _end)) {
      return true;
    }

    if (str == _str){
      return false;
    }
  }

  return true;
}

void reverse(const char *str) {
  int length, c;
  char *begin, *end, temp;

  length = strlen(str);
  begin = str;
  end = str;

  for (c = 0; c < length - 1; c++)
    end++;

    for (c = 0; c < length/2; c++) {
      temp = *end;
      *end = *begin;
      *begin = temp;

      begin++;
      end--;
   }
}

char* itoa(int num, char* str, int base) {
  int i = 0;
  bool isNegative = false;

  // Handle 0 explicitly, otherwise empty string is printed for 0
  if (num == 0) {
    str[i++] = '0';
    str[i] = '\0';

    return str;
  }

  // Negative numbers are handled only with base 10. Otherwise numbers are considered unsigned.
  if (num < 0 && base == 10) {
    isNegative = true;
    num = -num;
  }

  // Process individual digits
  while (num != 0) {
    int rem = num % base;
    str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
    num = num/base;
  }

  // If number is negative, append '-'
  if (isNegative) {
    str[i++] = '-';
  }

  str[i] = '\0'; // Append string terminator

  // Reverse the string
  reverse(str);

  return str;
}
