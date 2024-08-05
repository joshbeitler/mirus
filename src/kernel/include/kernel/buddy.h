#pragma once

#include <stdint.h>
#include <stddef.h>

#define MAX_ORDER 11

// TODO: actually derive this from the page size
#define PAGE_SHIFT 12 // Derived from PAGE_SIZE

/**
 * Bitmap of physical memory pages. 2D array of uint64_t, where the first
 * dimension is the order of the page size and the second dimension is the
 * number of uint64_t words needed to represent all pages in that order.
 * Each bit in the uint64_t represents the availability of a page,
 * 1 = free, 0 = used.
 */
typedef uint64_t BuddyBitmap[MAX_ORDER + 1][1 + (1 << MAX_ORDER) / 64];
typedef uint64_t* BuddyBitmapOrder;

typedef struct {
  uintptr_t start_address;
  BuddyBitmap bitmap;
} BuddyAllocator;

/**
 * Utility function to set a bit in a BuddyBitmap
 *
 * @param bitmap The sub-bitmap to modify bits in
 * @param bit The bit to set
 */
void buddy_allocator_set_bit(BuddyBitmapOrder bitmap, int bit);

/**
 * utility function to clear a bit in a buddybitmap
 *
 * @param bitmap the sub-bitmap to modify bits in
 * @param bit the bit to clear
 */
void buddy_allocator_clear_bit(BuddyBitmapOrder bitmap, int bit);

/**
 * Utility function to check a bit in a BuddyBitmap
 *
 * @param bitmap The sub-bitmap to check for bits in
 * @param bit The bit to check
 *
 * @return Status of bit
 */
int buddy_allocator_test_bit(BuddyBitmapOrder bitmap, int bit);

/**
 * Finds first set bit in the bitmap. This allows us to use 1s to represent
 * free pages and 0s to represent used pages, and therefore we can find the
 * first free page by finding the first set bit super quickly with ffs.
 *
 * @param bitmap The sub-bitmap to search
 * @param size The size of the bitmap in 64-bit words
 *
 * @return The index of the first set bit, or -1 if no set bit is found
 */
int buddy_allocator_find_first_set(BuddyBitmapOrder bitmap, int size);

/**
 * Utility function to find the first usable free block in a given order
 *
 * @param allocator The BuddyAllocator to search the bitmap of
 * @param order The order of the block to find
 *
 * @return The bit of the first free block, or -1 if no free block is found
 */
int buddy_allocator_find_free_block(BuddyAllocator* allocator, int order);

/**
 * Utility function to find which order a given size will fit into
 *
 * @param size The size to find the order for
 *
 * @return The order that the size fits into
 */
int buddy_allocator_get_order(size_t size);

/**
 * Initliualizes a BuddyAllocator with a given start address
 *
 * @param allocator The BuddyAllocator to initialize
 * @param start_address The start address of the memory to manage
 */
void buddy_allocator_init(BuddyAllocator *allocator, uintptr_t start_address);

/**
 * Allocates a block of memory of a given size
 *
 * @param allocator The BuddyAllocator to allocate from
 * @param size The size of the block to allocate
 *
 * @return The address of the allocated block, or NULL if no block is available
 */
uintptr_t buddy_allocator_alloc(BuddyAllocator *allocator, size_t size);

/**
 * Frees a block of memory at a given address
 *
 * @param allocator The BuddyAllocator to free memory from
 * @param address The address of the block to free
 */
void buddy_allocator_free(BuddyAllocator *allocator, uintptr_t address, size_t size);

/**
 * Dumps the bitmap of a BuddyAllocator to the console
 *
 * @param allocator The BuddyAllocator to dump the bitmap of
 */
void buddy_allocator_dump_bitmap(BuddyAllocator *allocator);