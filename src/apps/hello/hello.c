#include <stdint.h>
#include <stddef.h>

// Define syscall numbers
#define SYS_WRITE 1
#define STDOUT_FILENO 1

// Function to perform syscall
static inline long syscall(long n, long a1, long a2, long a3) {
  long ret;
  asm volatile (
    "syscall"
    : "=a"(ret)
    : "a"(n), "D"(a1), "S"(a2), "d"(a3)
    : "rcx", "r11", "memory"
  );
  return ret;
}

// Simple string length function
size_t strlen(const char* str) {
  size_t len = 0;
  while (str[len]) {
    len++;
  }
  return len;
}

// Entry point
void _start() {
  const char msg[] = "Hello, World from userspace!\n";
  syscall(SYS_WRITE, STDOUT_FILENO, (long)msg, strlen(msg));

  // Exit syscall (you may need to implement this)
  // syscall(SYS_EXIT, 0, 0, 0);

  // If no exit syscall, just loop forever
  while(1);
}
