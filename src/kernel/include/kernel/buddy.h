#pragma once

#include <stdint.h>
#include <stddef.h>

#define MAX_ORDER 11

/**
 * Buddy allocator for physical memory.
 *
 * Implemented as a linked-list approach. The allocator will
 * have free lists of available nodes. Each list will represent
 * an order.
 */

typedef struct BuddyBlock {
  struct BuddyBlock* next;
  size_t size;
} BuddyBlock;

typedef struct {
  uintptr_t start_address;
  size_t pool_size;
  BuddyBlock* free_lists[MAX_ORDER + 1]; // MAX_ORDER + 1 because this ensures we have a free list from order 0 to MAX_ORDER inclusive
} BuddyAllocator;


uintptr_t buddy_allocator_allocate(size_t size);
void buddy_allocator_free(uintptr_t address);

void buddy_allocator_init(
  BuddyAllocator* allocator,
  uintptr_t start_address,
  size_t pool_size
);
