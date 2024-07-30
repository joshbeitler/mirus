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

void read_memory_map(uint64_t entry_count, struct limine_memmap_entry **entries);
