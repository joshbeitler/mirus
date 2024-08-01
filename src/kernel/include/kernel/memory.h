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
 * Get a human-readable name for a memory map entry type
 *
 * @param type Memory map entry type
 * @return Human-readable name for the type
 */
const char* get_memmap_type_string(uint64_t type);

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
 *
 * TODO: This should be replaced with a proper memory map API that abstracts
 *      the bootloader interface
 */
void pmm_initialize(
  uint64_t entry_count,
  struct limine_memmap_entry **entries,
  struct limine_kernel_address_response *kernel_address_response,
  struct limine_kernel_file_response *kernel_file_response
);

/**
 * Global variables for holding memory sizes of different types. Initialized by
 * the pmm_initialize function.
 */
static uint64_t total_memory;
static uint64_t usable_memory;

/**
 * Global variables to tell the kernel where the kernel is loaded. Initialized
 * by the pmm_initialize function.
 */
static uintptr_t kernel_start;
static uintptr_t kernel_end;

/**
 * Default page size
 */
#define PAGE_SIZE 4096

/**
 * Page read/write flags
 */
#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)

/**
 * Number of pages represented by each bitmap word
 */
#define PAGES_PER_WORD 64
