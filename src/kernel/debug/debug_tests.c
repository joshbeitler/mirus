#include <printf/printf.h>

#include <kernel/debug.h>
#include <kernel/syscalls.h>
#include <kernel/pmm.h>

void debug_test_syscalls() {
  printf_("Running system calls test\n");

  const char test_str[] = "Hello, Kernel World!";
  int test_fd = 1;  // Assume 1 is stdout

  // Create syscall arguments
  SystemCallArgs args = {
    .args = {test_fd, (uint64_t)test_str, sizeof(test_str) - 1, 0, 0, 0}
  };

  // Call syscall dispatcher directly
  printf_("  Trying write syscall\n");
  SystemCallReturn result = syscall_handler(SYSCALL_WRITE, &args);

  // Check results
  if (result.error == SYSCALL_SUCCESS) {
    printf_("  Write syscall test successful!\n");
    printf_("  Bytes written: %ld (expected %zu)\n", result.value, sizeof(test_str) - 1);
  } else {
    printf_("  Write syscall test failed with error: %d\n", result.error);
  }

  // Call syscall dispatcher directly
  SystemCallArgs args2 = {
    .args = {0, 0, 0, 0, 0, 0}
  };
  printf_("  Trying invalid syscall\n");
  SystemCallReturn result2 = syscall_handler(15, &args2);
  printf_("  Invalid syscall test failed with error: %d\n", result2.error);

  printf_("Done running system calls test\n");
}

void debug_test_exceptions() {
  printf_("Running exceptions test\n");
  log_message(&kernel_debug_logger, LOG_DEBUG, "kernel", "Running exceptions test\n");

  // Try the exception handler
  int a = 10;
  int b = 0;
  int c = a / b; // This will generate a Division By Zero exception

  log_message(&kernel_debug_logger, LOG_DEBUG, "kernel", "Result of division: %d\n", c);
  printf_("Are we still here?\n");
}

void debug_test_buddy_allocator() {

}
