#include <stdint.h>
#include <stddef.h>

#include <printf/printf.h>
#include <libk/string.h>

#include <kernel/paging.h>
#include <kernel/debug.h>
#include <kernel/buddy.h>

// Used for printing buddy allocator bitmap in a table
#define STATUS_WIDTH 72

void buddy_allocator_set_bit(BuddyBitmapOrder bitmap, int bit) {
  bitmap[bit / 64] |= (1ULL << (bit % 64));
}

void buddy_allocator_clear_bit(BuddyBitmapOrder bitmap, int bit) {
  bitmap[bit / 64] &= ~(1ULL << (bit % 64));
}

int buddy_allocator_test_bit(BuddyBitmapOrder bitmap, int bit) {
  return (bitmap[bit / 64] & (1ULL << (bit % 64))) != 0;
}

int buddy_allocator_find_first_set(BuddyBitmapOrder bitmap, int size) {
  for (int i = 0; i < size; i++) {
    if (bitmap[i] != 0) {
      // Uses builtin ffs function to find the first set bit super quickly
      return i * 64 + __builtin_ffsll(bitmap[i]) - 1;
    }
  }

  return -1;  // No set bit found
}

int buddy_allocator_find_free_block(BuddyAllocator* allocator, int order) {
  for (int i = order; i <= MAX_ORDER; i++) {
    int bit = buddy_allocator_find_first_set(
      allocator->bitmap[i],
      1 + (1 << MAX_ORDER) / 64
    );

    if (bit != -1) {
      return bit;
    }
  }

  return -1;
}

int buddy_allocator_get_order(size_t size) {
  int order = 0;
  size = (size - 1) >> PAGE_SHIFT;

  while (size > 0) {
    order++;
    size >>= 1;
  }

  return order;
}

void buddy_allocator_init(BuddyAllocator *allocator, uintptr_t start_address) {
  // Set start address
  allocator->start_address = start_address;

  // Set bitmap to all free
  memset(allocator->bitmap, 0xFF, sizeof(allocator->bitmap));
}

uintptr_t buddy_allocator_alloc(BuddyAllocator *allocator, size_t size) {
  int order = buddy_allocator_get_order(size);
  int bit = buddy_allocator_find_free_block(allocator, order);

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_DEBUG,
      "PMM: Allocating %zu bytes (order %d)\n",
      size,
      order
    );
  }

  if (bit == -1) {
    if (DEBUG) {
      log_message(
        &kernel_debug_logger,
        LOG_ERROR,
        "  No free block found for order: %d\n",
        order
      );
    }

    return (uintptr_t)NULL;
  }

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_DEBUG,
      "  Free block found at bit %d for order: %d\n",
      bit,
      order
    );
  }

  // Split larger blocks if necessary
  for (int i = MAX_ORDER; i > order; i--) {
    if (buddy_allocator_test_bit(allocator->bitmap[i], bit >> (i - order))) {
      buddy_allocator_clear_bit(allocator->bitmap[i], bit >> (i - order));
      buddy_allocator_set_bit(allocator->bitmap[i-1], (bit >> (i - order - 1)) ^ 1);
    }
  }

  // Mark the allocated block as used
  buddy_allocator_clear_bit(allocator->bitmap[order], bit);

  // Calculate the physical address
  uintptr_t address = (uintptr_t)bit << (PAGE_SHIFT + order);

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_DEBUG,
      "  Allocated %zu bytes (requested: %zu bytes) at 0x%016lx\n",
      (size_t)1 << (PAGE_SHIFT + order),
      size,
      address
    );
  }

  return address;
}

void buddy_allocator_free(
  BuddyAllocator *allocator,
  uintptr_t address,
  size_t size
) {
  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_DEBUG,
      "PMM: Freeing %zu bytes at address 0x%016lx\n",
      size,
      address
    );
  }

  // Calculate the order of the block being freed
  int order = buddy_allocator_get_order(size);

  // Calculate the bit index in the bitmap
  int bit = (address >> (PAGE_SHIFT + order));

  while (order <= MAX_ORDER) {
    // Set this bit as free
    buddy_allocator_set_bit(allocator->bitmap[order], bit);

    if (DEBUG) {
      log_message(
        &kernel_debug_logger,
        LOG_DEBUG,
        "  Freed block at bit %d for order %d\n",
        bit,
        order
      );
    }

    // If we're at the highest order or if the buddy is not free, we're done
    if (
      order == MAX_ORDER ||
      !buddy_allocator_test_bit(allocator->bitmap[order], bit ^ 1)
    ) {
      return;
    }

    // If we're here, we can merge with the buddy
    if (DEBUG) {
      log_message(
        &kernel_debug_logger,
        LOG_DEBUG,
        "  Merging blocks at bits %d and %d for order %d\n",
        bit,
        bit ^ 1,
        order
      );
    }

    // Move to the next order
    bit >>= 1;
    order++;
  }

  // We should never reach here, but if we do, it's an error
  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_ERROR,
      "PMM: Error in pmm_free - reached end of function unexpectedly\n"
    );
  }
}

void buddy_allocator_dump_bitmap(BuddyAllocator *allocator) {
  if (DEBUG) {
    log_message(&kernel_debug_logger, LOG_DEBUG, "PMM: Buddy bitmap state\n");
  }

  printf_("\nBuddy Allocator Status:\n");
  printf_("+-------+----------+-------------+------------+--------------------------------------------------------------------------+\n");
  printf_("| Order | Blocks   | Block Size  | Total Mem  | Status (1=Free)                                                          |\n");
  printf_("+-------+----------+-------------+------------+--------------------------------------------------------------------------+\n");

  for (int order = 0; order <= MAX_ORDER; order++) {
    int blocks_in_order = 1 << (MAX_ORDER - order);
    size_t block_size = (size_t)PAGE_SIZE << order;
    size_t total_memory = block_size * blocks_in_order;

    printf_(
      "| %5d | %8d | %7zu KiB | %6zu MiB | ",
      order,
      blocks_in_order,
      block_size / 1024,
      total_memory / (1024 * 1024)
    );

    if (DEBUG) {
      log_message(
        &kernel_debug_logger,
        LOG_DEBUG,
        "  {order=%2d, blocks_in_order=%4d, block_size=%4d kib, total_mem=%d mib}\n",
        order,
        blocks_in_order,
        block_size / 1024,
        total_memory / (1024 * 1024)
      );
    }

    char status[STATUS_WIDTH + 1];  // +1 for null terminator
    memset(status, ' ', STATUS_WIDTH);
    status[STATUS_WIDTH] = '\0';

    int words_needed = (blocks_in_order + 63) / 64;
    int bit_count = 0;

    // TODO: we need to debug log each order's bitmap
    //       to the serial log

    for (int word = 0; word < words_needed && bit_count < STATUS_WIDTH; word++) {
      uint64_t current_word = allocator->bitmap[order][word];

      for (int bit = 0; bit < 64 && bit_count < STATUS_WIDTH; bit++) {
        status[bit_count++] = (current_word & (1ULL << bit)) ? '1' : '0';

        if (bit_count % 8 == 0 && bit_count < STATUS_WIDTH) {
          status[bit_count++] = ' ';
        }
      }
    }

    if (blocks_in_order > STATUS_WIDTH) {
      status[STATUS_WIDTH - 3] = '.';
      status[STATUS_WIDTH - 2] = '.';
      status[STATUS_WIDTH - 1] = '.';
    }

    printf_("%s |\n", status);
  }

  printf_("+-------+----------+-------------+------------+--------------------------------------------------------------------------+\n");
}
