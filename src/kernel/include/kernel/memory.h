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
 */
const char* get_memmap_type_string(uint64_t type);

/**
 * Utility function that formats a memory size as a human-readable string
 */
char* format_memory_size(uint64_t size, char* buffer, size_t buffer_size);

/**
 * Read the memory map from the bootloader
 *
 * TODO: This should be replaced with a proper memory map API that abstracts
 *      the bootloader interface
 */
// void read_memory_map(uint64_t entry_count, struct limine_memmap_entry **entries);
void read_memory_map(uint64_t entry_count, struct limine_memmap_entry **entries, struct limine_kernel_address_response *kernel_address_response, struct limine_kernel_file_response *kernel_file_response);


static uint64_t total_memory;
static uint64_t usable_memory;

void free_page(uintptr_t page);
uintptr_t allocate_page(void);
void init_page_allocator();
void map_page(uint64_t virtual_address, uint64_t physical_address, uint64_t flags);
void invlpg(uint64_t virtual_address);
uint64_t* get_pml4_address(void);

static uint64_t *page_bitmap;  // Pointer to our bitmap
static uint64_t total_pages;   // Total number of pages in the system
static uint64_t free_pages;    // Number of free pages
static uintptr_t kernel_start;
static uintptr_t kernel_end;
#define PAGE_SIZE 4096
#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGES_PER_WORD 64  // Number of pages represented by each bitmap word
