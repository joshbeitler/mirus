#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Returns a reference to the kernel stack pointer that was set up by
 * the boot loader.
 *
 * TODO: Will be eventually replaced when we set up our own stack.
 */
static inline uintptr_t get_kernel_stack_ptr() {
	uintptr_t stack_ptr;
	asm volatile("mov %%rsp, %0" : "=r"(stack_ptr));
	return stack_ptr;
}
