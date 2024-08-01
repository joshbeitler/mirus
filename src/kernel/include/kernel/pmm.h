#pragma once

#include <stdint.h>
#include <stddef.h>

#include <limine/limine.h>

/**
 * Human-readable names for memory map entry types
 */
static const char* const memmap_type_strings[] = {
  "Usable",
  "Reserved",
  "ACPI Reclaimable",
  "ACPI NVS",
  "Bad Memory",
  "Bootloader Reclaimable",
  "Kernel and Modules",
  "Framebuffer"
};

#define MEMMAP_TYPE_COUNT (sizeof(memmap_type_strings) / sizeof(memmap_type_strings[0]))

/**
 * Utility function that formats a memory size as a human-readable string
 *
 * @param size Size in bytes
 * @param buffer Buffer to write the formatted string to
 * @param buffer_size Size of the buffer
 * @return Pointer to the buffer
 */
char* format_memory_size(uint64_t size, char* buffer, size_t buffer_size);

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
 */
uintptr_t pmm_alloc(size_t size);

/**
 * Free a previously allocated block of physical memory. We use a buddy
  * allocator to manage physical memory.
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
