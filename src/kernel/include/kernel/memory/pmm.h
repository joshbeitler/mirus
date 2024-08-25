#pragma once

#include "buddy_allocator.h"
#include <stddef.h>
#include <stdint.h>

#include <limine/limine.h>

#include <kernel/memory/buddy_allocator.h>

/**
 * Represents a buddy allocator in an intrusive linked list. This is used to
 * allow multuiple buddy allocators to manage a single memory zone without
 * needing free/alloc.
 */
typedef struct BuddyAllocatorNode {
	BuddyAllocator allocator;
	struct BuddyAllocatorNode *next;
} BuddyAllocatorNode;

/**
 * Represents a memory zone. A memory zone is a contiguous block of physical
 * memory. Each memory zone has its a list of buddy allocators to manage, stored
 * as an intrusive linked list. Zones themeselves are also stored as a intrusive
 * linked list.
 */
typedef struct MemoryZone {
	uintptr_t start_address;
	size_t size;
	BuddyAllocatorNode *allocators;
	struct MemoryZone *next;
} MemoryZone;

/**
 * Intrusive linked list of memory zones.
 */
static MemoryZone *memory_zones;

/**
 * Read and interpret the memory map from the bootloader
 *
 * @param entry_count Number of memory map entries
 * @param entries Pointer to the memory map entries
 * @param kernel_address_response Pointer to the kernel address response
 * @param kernel_file_response Pointer to the kernel file response. Used to get
 *        the kernel file size.
 * @param hhdm_response Pointer to the HHDM response. Used to get the HHDM
 *        offset.
 */
void pmm_initialize(
	uint64_t entry_count,
	struct limine_memmap_entry **entries,
	struct limine_kernel_address_response *kernel_address_response,
	struct limine_hhdm_response *hhdm_response // not ideal to pass here
);

/**
 * Allocate a block of physical memory. We use a buddy allocator to manage
 * physical memory.
 *
 * @param size Size of the block to allocate
 * @return The address of the allocated block
 */
uintptr_t pmm_alloc(size_t size);

/**
 * Free a previously allocated block of physical memory. We use a buddy
 * allocator to manage physical memory.
 *
 * @param addr Address of the block to free
 */
void pmm_free(uintptr_t addr);

/**
 * Print the current state of the memory manager, for debugging purposes
 */
void pmm_debug_print_state();
