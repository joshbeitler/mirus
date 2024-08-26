#pragma once

#include <stddef.h>
#include <stdint.h>

#define MAX_ORDER 11

/**
 * BuddyBlock represents a block of memory that is free.
 *
 * The size of the block is 2^order bytes, with the smallest block size being
 * PAGE_SIZE bytes.
 */
typedef struct BuddyBlock {
	struct BuddyBlock *next;
	size_t size;
} BuddyBlock;

/**
 * Buddy allocator for physical memory.
 *
 * Implemented as a linked-list approach. The allocator will have free lists of
 * available nodes. Each list will represent an order.
 */
typedef struct {
	/**
	 * Page-aligned virtual start address of the memory pool. Use phys_to_virt
	 * to convert addresses from the memory map to virtual addresses.
	 */
	uintptr_t start_address;

	/**
	 * The size of the memory pool in bytes. The buddy allocator will try
	 * to efficiently allocate memory from this pool.
	 */
	size_t pool_size;

	/**
	 * MAX_ORDER + 1 because this ensure we have a free list from order 0 to
	 * MAX_ORDER inclusive
	 */
	BuddyBlock *free_lists[MAX_ORDER + 1];
} BuddyAllocator;

/**
 * Allocate a memory block of the given size.
 *
 * @param allocator The buddy allocator to allocate the memory block from.
 * @param size The size of the memory block to allocate.
 * @return The address of the allocated memory block, or 0 if no memory block
 */
uintptr_t buddy_allocator_allocate(BuddyAllocator *allocator, size_t size);

/**
 * Free the memory block at the given address.
 *
 * @param allocator The buddy allocator to free the memory block from.
 * @param address The address of the memory block to free.
 */
void buddy_allocator_free(BuddyAllocator *allocator, uintptr_t address);

/**
 * Initialize the buddy allocator with the given memory pool at a given address.
 *
 * @param allocator The buddy allocator to initialize.
 * @param start_address The page-aligned virtual start address of the memory
 *        pool.
 * @param pool_size The size of the memory pool in bytes.
 */
void buddy_allocator_initialize(
	BuddyAllocator *allocator, uintptr_t start_address, size_t pool_size
);

/**
 * Print the state of the buddy allocator, showing free lists and their
 * available addresses for each order.
 *
 * @param allocator The buddy allocator to print the state of.
 */
void buddy_allocator_debug_state(BuddyAllocator *allocator);
