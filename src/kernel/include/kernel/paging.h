#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Default page size
 */
#define PAGE_SIZE 4096

/**
 * Read the value of the CR2 register
 */
static inline uint64_t read_cr2() {
	uint64_t value;
	__asm__ volatile("mov %%cr2, %0" : "=r"(value));
	return value;
}

/**
 * Convert a virtual address to a physical address using the higher-half direct
 * memory offset we get from the bootloader
 */
static inline void *phys_to_virt(uint64_t phys, uint64_t hhdm_offset) {
	return (void *)(phys + hhdm_offset);
}
