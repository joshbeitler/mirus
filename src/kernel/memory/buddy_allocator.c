#include <stddef.h>
#include <stdint.h>

#include <jems/jems.h>
#include <libk/string.h>
#include <printf/printf.h>

#include <kernel/debug.h>
#include <kernel/memory/buddy_allocator.h>
#include <kernel/memory/paging.h>

#define JEMS_MAX_LEVEL 10

static void jems_writer(char ch, uintptr_t arg) {
	logger_t *logger = (logger_t *)arg;
	char str[2] = {ch, '\0'};
	log_stream_data(logger, str, 1);
}

/**
 * Utility function to get the proper order for a given size
 */
static int find_order(size_t size) {
	int order = 0;
	size_t block_size = PAGE_SIZE;

	while (block_size < size && order < MAX_ORDER) {
		block_size *= 2;
		order++;
	}

	return order;
}

/**
 * Utility function to split a block into two smaller blocks
 */
static void split_block(BuddyAllocator *allocator, int order) {
	if (order == 0 || allocator->free_lists[order] == NULL) {
		return;
	}

	BuddyBlock *block = allocator->free_lists[order];
	allocator->free_lists[order] = block->next;

	size_t half_size = PAGE_SIZE * (1 << (order - 1));
	BuddyBlock *buddy = (BuddyBlock *)((uintptr_t)block + half_size);

	block->size = half_size;
	buddy->size = half_size;

	block->next = buddy;
	buddy->next = allocator->free_lists[order - 1];
	allocator->free_lists[order - 1] = block;
}

/**
 * Utility function to find the buddy of a block
 */
static uintptr_t find_buddy(uintptr_t block_addr, size_t block_size) {
	return block_addr ^ block_size;
}

uintptr_t buddy_allocator_allocate(BuddyAllocator *allocator, size_t size) {
	int order = find_order(size);
	if (order > MAX_ORDER) {
		return 0; // Requested size is too large
	}

	// Find the smallest available block that fits the requested size
	for (int i = order; i <= MAX_ORDER; i++) {
		if (allocator->free_lists[i] != NULL) {
			// Split larger blocks if necessary
			while (i > order) {
				split_block(allocator, i);
				i--;
			}

			// Allocate the block
			BuddyBlock *block = allocator->free_lists[i];
			allocator->free_lists[i] = block->next;
			return (uintptr_t)block;
		}
	}

	return 0; // No suitable block found
}

void buddy_allocator_free(BuddyAllocator *allocator, uintptr_t address) {
	BuddyBlock *block = (BuddyBlock *)address;
	int order = find_order(block->size);

	while (order < MAX_ORDER) {
		uintptr_t buddy_addr = find_buddy(address, block->size);
		BuddyBlock *buddy = (BuddyBlock *)buddy_addr;

		// Check if the buddy is free and of the same size
		int found = 0;
		BuddyBlock *prev = NULL;
		BuddyBlock *curr = allocator->free_lists[order];

		while (curr != NULL) {
			if (curr == buddy) {
				found = 1;
				break;
			}

			prev = curr;
			curr = curr->next;
		}

		if (!found) {
			break; // Buddy not free, can't merge
		}

		// Remove buddy from free list
		if (prev == NULL) {
			allocator->free_lists[order] = curr->next;
		} else {
			prev->next = curr->next;
		}

		// Merge blocks
		if (buddy_addr < address) {
			address = buddy_addr;
		}

		block = (BuddyBlock *)address;
		block->size *= 2;
		order++;
	}

	// Add merged block to appropriate free list
	block->next = allocator->free_lists[order];
	allocator->free_lists[order] = block;
}

void buddy_allocator_initialize(
	BuddyAllocator *allocator, uintptr_t start_address, size_t pool_size
) {
	if (DEBUG) {
		log_message(
			&kernel_debug_logger,
			LOG_INFO,
			"memory_manager",
			"Initializing buddy allocator at 0x%016llx with size %d bytes\n",
			start_address,
			pool_size
		);
	}

	// Set up allocator struct
	allocator->start_address = start_address;
	allocator->pool_size = pool_size;

	if (DEBUG) {
		log_message(
			&kernel_debug_logger,
			LOG_DEBUG,
			"memory_manager",
			"Initializing free lists to null\n"
		);
	}

	// Initialize free lists to NULL
	for (int i = 0; i <= MAX_ORDER; i++) {
		allocator->free_lists[i] = NULL;
	}

	size_t remaining_size = pool_size;
	uintptr_t current_address = start_address;

	if (DEBUG) {
		log_message(
			&kernel_debug_logger,
			LOG_DEBUG,
			"memory_manager",
			"Creating memory blocks\n"
		);
	}

	while (remaining_size >= PAGE_SIZE) {
		int order = MAX_ORDER;
		size_t block_size = PAGE_SIZE << MAX_ORDER;

		// Find the largest block size that fits in the remaining space
		while (order >= 0 && block_size > remaining_size) {
			order--;
			block_size >>= 1;
		}

		// Create and add the block to the appropriate free list
		BuddyBlock *block = (BuddyBlock *)current_address;
		block->size = block_size;
		block->next = allocator->free_lists[order];
		allocator->free_lists[order] = block;

		if (DEBUG) {
			log_message(
				&kernel_debug_logger,
				LOG_DEBUG,
				"memory_manager",
				"Created block of order %d at address 0x%016llx\n",
				order,
				(unsigned long long)current_address
			);
		}

		current_address += block_size;
		remaining_size -= block_size;
	}

	if (DEBUG) {
		log_message(
			&kernel_debug_logger,
			LOG_INFO,
			"memory_manager",
			"Buddy allocator initialization complete\n"
		);
	}
}

void buddy_allocator_debug_state(BuddyAllocator *allocator) {
	static jems_level_t jems_levels[JEMS_MAX_LEVEL];
	static jems_t jems;
	char buffer[64];

	// Start the log stream
	log_stream_start(
		&kernel_debug_logger,
		LOG_DEBUG,
		"memory_manager",
		"Buddy allocator state"
	);

	jems_init(
		&jems,
		jems_levels,
		JEMS_MAX_LEVEL,
		jems_writer,
		(uintptr_t)&kernel_debug_logger
	);

	jems_object_open(&jems);
	jems_key_integer(&jems, "total_memory_size", allocator->pool_size);
	jems_key_integer(&jems, "total_pages", allocator->pool_size / PAGE_SIZE);

	// Print information for each order
	jems_key_array_open(&jems, "orders");
	for (int order = 0; order <= MAX_ORDER; order++) {
		size_t block_size = PAGE_SIZE * (1 << order);
		int block_count = 0;
		size_t total_free_memory = 0;

		// Count blocks and total free memory in this order
		BuddyBlock *current = allocator->free_lists[order];
		while (current != NULL) {
			block_count++;
			total_free_memory += block_size;
			current = current->next;
		}

		jems_object_open(&jems);
		jems_key_integer(&jems, "order", order);
		jems_key_integer(&jems, "block_size", block_size);
		jems_key_integer(&jems, "free_blocks", block_count);
		jems_key_integer(&jems, "total_free_memory", total_free_memory);

		jems_key_array_open(&jems, "blocks");

		// Print addresses of free blocks
		current = allocator->free_lists[order];
		if (current != NULL) {
			while (current != NULL) {
				snprintf_(buffer, sizeof(buffer), "%p", (void *)current);
				jems_string(&jems, buffer);
				current = current->next;
			}
		}

		jems_array_close(&jems);
		jems_object_close(&jems);
	}

	jems_array_close(&jems);
	jems_object_close(&jems);

	// End the log stream
	log_stream_end(&kernel_debug_logger);

	// Calculate and print total free memory
	size_t total_free = 0;
	for (int order = 0; order <= MAX_ORDER; order++) {
		BuddyBlock *current = allocator->free_lists[order];
		while (current != NULL) {
			total_free += PAGE_SIZE * (1 << order);
			current = current->next;
		}
	}

	log_message(
		&kernel_debug_logger,
		LOG_DEBUG,
		"memory_manager",
		"Total free memory: %zu bytes\n",
		total_free
	);

	log_message(
		&kernel_debug_logger,
		LOG_DEBUG,
		"memory_manager",
		"Total allocated memory: %zu bytes\n",
		allocator->pool_size - total_free
	);
}
