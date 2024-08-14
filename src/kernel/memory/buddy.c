#include <stdint.h>
#include <stddef.h>

#include <printf/printf.h>
#include <libk/string.h>
#include <jems/jems.h>

#include <kernel/paging.h>
#include <kernel/debug.h>
#include <kernel/buddy.h>

uintptr_t buddy_allocator_allocate(size_t size) {

}

void buddy_allocator_free(uintptr_t address) {

}

void buddy_allocator_init(
  BuddyAllocator* allocator,
  uintptr_t start_address,
  size_t pool_size
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
      LOG_INFO,
      "memory_manager",
      "Initializing free lists to null\n",
      start_address
    );
  }

  // Initialize free lists to NULL
  for (int i = 0; i<= MAX_ORDER; i++) {
    allocator->free_lists[i] = NULL;
  }

  size_t num_pages = pool_size / PAGE_SIZE;
  uintptr_t current_address = start_address;

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "memory_manager",
      "Initializing %d pages in free lists\n",
      num_pages
    );
  }

  // Create single-page blocks
  for (size_t i = 0; i < num_pages; i++) {
    BuddyBlock* block = (BuddyBlock*)current_address;
    block->size = PAGE_SIZE;
    block->next = allocator->free_lists[0];
    current_address += PAGE_SIZE;
  }
}
