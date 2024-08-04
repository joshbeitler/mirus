#pragma once

#include <stdint.h>
#include <stddef.h>

#include <limine/limine.h>

/**
 * Read and interpret the memory map from the bootloader
 *
 * @param entry_count Number of memory map entries
 * @param entries Pointer to the memory map entries
 * @param kernel_address_response Pointer to the kernel address response
 * @param kernel_file_response Pointer to the kernel file response. Used to get
 *        the kernel file size.
 */
void pmm_initialize(
  uint64_t entry_count,
  struct limine_memmap_entry **entries,
  struct limine_kernel_address_response *kernel_address_response,
  struct limine_kernel_file_response *kernel_file_response
);

#define MAX_ORDER 11

/**
 * Allocate a block of physical memory. We use a buddy allocator to manage
 * physical memory.
 *
 * @param size Size of the block to allocate
 * @return The physical address of the allocated block
 */
uintptr_t pmm_alloc(size_t size);

/**
 * Free a previously allocated block of physical memory. We use a buddy
 * allocator to manage physical memory.
 *
 * @param addr Physical address of the block to free
 * @param size Size of the block to free
 */
void pmm_free(uintptr_t addr, size_t size);

/**
 * Get the total amount of physical memory
 */
size_t pmm_get_total_memory();

/**
 * Get the amount of free physical memory
 */
size_t pmm_get_free_memory();

/**
 * Print the current state of the memory manager, for debugging purposes
 */
void pmm_debug_print_state();
