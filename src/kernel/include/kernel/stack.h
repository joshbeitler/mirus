#pragma once

#include <stddef.h>
#include <stdint.h>

#include <kernel/memory/paging.h>

/**
 * Returns a reference to the kernel stack pointer.
 */
static inline uintptr_t get_kernel_stack_ptr() {
	uintptr_t stack_ptr;
	asm volatile("mov %%rsp, %0" : "=r"(stack_ptr));
	return stack_ptr;
}
