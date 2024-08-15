#include <printf/printf.h>

#include <kernel/debug.h>
#include <kernel/memory/pmm.h>
#include <kernel/syscalls.h>

void debug_test_syscalls() {
	printf_("Running system calls test\n");

	const char test_str[] = "Hello, Kernel World!";
	int test_fd = 1; // Assume 1 is stdout

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
		printf_(
			"  Bytes written: %ld (expected %zu)\n",
			result.value,
			sizeof(test_str) - 1
		);
	} else {
		printf_("  Write syscall test failed with error: %d\n", result.error);
	}

	// Call syscall dispatcher directly
	SystemCallArgs args2 = {.args = {0, 0, 0, 0, 0, 0}};
	printf_("  Trying invalid syscall\n");
	SystemCallReturn result2 = syscall_handler(15, &args2);
	printf_("  Invalid syscall test failed with error: %d\n", result2.error);

	printf_("Done running system calls test\n");
}

void debug_test_exceptions() {
	printf_("Running exceptions test\n");
	log_message(
		&kernel_debug_logger, LOG_DEBUG, "kernel", "Running exceptions test\n"
	);

	// Try the exception handler
	int a = 10;
	int b = 0;
	int c = a / b; // This will generate a Division By Zero exception

	log_message(
		&kernel_debug_logger, LOG_DEBUG, "kernel", "Result of division: %d\n", c
	);
	printf_("Are we still here?\n");
}

void debug_test_buddy_allocator() {
	// Allocate some memory
	uintptr_t addr1 = pmm_alloc(4096);	// Allocate a single page
	uintptr_t addr2 = pmm_alloc(8192);	// Allocate two pages
	uintptr_t addr3 = pmm_alloc(16384); // Allocate four pages

	log_message(
		&kernel_debug_logger,
		LOG_DEBUG,
		"memory_manager",
		"Allocated addresses: 0x%llx, 0x%llx, 0x%llx",
		addr1,
		addr2,
		addr3
	);

	// Free the allocated memory
	pmm_free(addr1);
	pmm_free(addr2);
	pmm_free(addr3);

	// Try to allocate a large block
	uintptr_t large_addr = pmm_alloc(131072); // 128 KB
	log_message(
		&kernel_debug_logger,
		LOG_DEBUG,
		"memory_manager",
		"Large allocation address: 0x%llx",
		large_addr
	);

	// Free the large block
	pmm_free(large_addr);

// Allocate many small blocks
#define NUM_BLOCKS 50
	uintptr_t addresses[NUM_BLOCKS];
	for (int i = 0; i < NUM_BLOCKS; i++) {
		addresses[i] = pmm_alloc(4096);
		log_message(
			&kernel_debug_logger,
			LOG_DEBUG,
			"memory_manager",
			"Small allocation %d: 0x%llx",
			i,
			addresses[i]
		);
	}

	// Free every other block
	for (int i = 0; i < NUM_BLOCKS; i += 2) {
		pmm_free(addresses[i]);
	}

	// Try to allocate a medium-sized block
	uintptr_t medium_addr = pmm_alloc(32768); // 32 KB
	log_message(
		&kernel_debug_logger,
		LOG_DEBUG,
		"memory_manager",
		"Medium allocation after freeing: 0x%llx",
		medium_addr
	);

	// Free all remaining blocks
	for (int i = 1; i < NUM_BLOCKS; i += 2) {
		pmm_free(addresses[i]);
	}
	pmm_free(medium_addr);

	// Print the final state of the allocator
	pmm_debug_print_state();
}
