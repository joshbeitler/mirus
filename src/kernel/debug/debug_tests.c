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

  log_message(
    &kernel_debug_logger,
    LOG_DEBUG,
    "kernel",
    "Starting buddy allocator test...\n"
  );

  printf_("Starting complex PMM test...\n");

  printf_("Initial state:\n");
  pmm_debug_print_state();

  printf_("Allocating 4096 bytes\n");

  // Allocate blocks of various sizes
  uintptr_t block1 = pmm_alloc(4096);    // 4 KiB (Order 0)
  // uintptr_t block2 = pmm_alloc(8192);    // 8 KiB (Order 1)
  // uintptr_t block3 = pmm_alloc(16384);   // 16 KiB (Order 2)
  // uintptr_t block4 = pmm_alloc(65536);   // 64 KiB (Order 4)
  // uintptr_t block5 = pmm_alloc(131072);  // 128 KiB (Order 5)
  pmm_debug_print_state();

  printf_("Freeing 4096 bytes\n");
  pmm_free(block1, 4096);

  // Free blocks in a specific order to test merging
  // pmm_free(block2, 8192);

  // pmm_free(block4, 65536);

  // Allocate a block that should fit in the hole left by block4
  // uintptr_t block6 = pmm_alloc(32768);   // 32 KiB (Order 3)

  // Free more blocks
  // pmm_free(block1, 4096);
  // pmm_free(block3, 16384);

  // Allocate a large block
  // uintptr_t block7 = pmm_alloc(262144);  // 256 KiB (Order 6)

  // Free remaining blocks
  // pmm_free(block5, 131072);
  // pmm_free(block6, 32768);
  // pmm_free(block7, 262144);

  printf_("Final state:\n");
  pmm_debug_print_state();

  printf_("Complex PMM test completed.\n");
}
