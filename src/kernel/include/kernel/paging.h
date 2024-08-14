#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Default page size
 */
#define PAGE_SIZE 4096

/**
 * Page read/write flags
 */
#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)

static inline void *phys_to_virt(uint64_t phys, uint64_t hhdm_offset) {
	return (void *)(phys + hhdm_offset);
}

// static inline uintptr_t virt_to_phys(void *virt_addr) {
// 	return (uintptr_t)virt_addr - hhdm_offset;
// }
