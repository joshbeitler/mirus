#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Default page size
 */
#define PAGE_SIZE 4096

#define ALIGN_UP(addr, align) (((addr) + (align) - 1) & ~((align) - 1))
#define ALIGN_UP_PAGE(addr) ALIGN_UP(addr, PAGE_SIZE)

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

/**
 * Convert a physical address to a virtual address using the higher-half direct
 * memory offset we get from the bootloader
 */
static inline uint64_t virt_to_phys(void *virt, uint64_t hhdm_offset) {
	return (uint64_t)virt - hhdm_offset;
}
