#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <printf/printf.h>
#include <limine/limine.h>
#include <libk/string.h>
#include <libk/math.h>

#include <hal/serial.h>

#include <kernel/pmm.h>
#include <kernel/paging.h>
#include <kernel/debug.h>

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
#define STATUS_WIDTH 72

/**
 * Internal helper variables
 */
static uintptr_t kernel_start, kernel_end;
static uint64_t total_memory, usable_memory;
static uint64_t total_frames;

/**
 * Bitmap of physical memory pages. 2D array of uint64_t, where the first
 * dimension is the order of the page size and the second dimension is the
 * number of uint64_t words needed to represent all pages in that order.
 * Each bit in the uint64_t represents the availability of a page,
 * 1 = free, 0 = used.
 */
static uint64_t bitmap[MAX_ORDER + 1][1 + (1 << MAX_ORDER) / 64];
#define PAGE_SHIFT 12 // Derived from PAGE_SIZE

/**
 * Utility function to set a bit in the memory map
 *
 * @param bitmap The bitmap to modify
 * @param bit The bit to set
 */
static void set_bit(uint64_t *bitmap, int bit) {
  bitmap[bit / 64] |= (1ULL << (bit % 64));
}

/**
 * Utility function to clear a bit in the memory map
 *
 * @param bitmap The bitmap to modify
 * @param bit The bit to clear
 */
static void clear_bit(uint64_t *bitmap, int bit) {
  bitmap[bit / 64] &= ~(1ULL << (bit % 64));
}

/**
 * Utility function to check a bit in the memory map
 *
 * @param bitmap The bitmap to modify
 * @param bit The bit to check
 */
static int test_bit(uint64_t *bitmap, int bit) {
  return (bitmap[bit / 64] & (1ULL << (bit % 64))) != 0;
}

/**
 * Finds first set bit in the bitmap. This allows us to use 1s to represent
 * free pages and 0s to represent used pages, and therefore we can find the
 * first free page by finding the first set bit super quickly with ffs.
 *
 * @param bitmap The bitmap to search
 * @param size The size of the bitmap in 64-bit words
 * @return The index of the first set bit, or -1 if no set bit is found
 */
static int find_first_set(uint64_t *bitmap, int size) {
  for (int i = 0; i < size; i++) {
    if (bitmap[i] != 0) {
      // Uses builtin ffs function to find the first set bit super quickly
      return i * 64 + __builtin_ffsll(bitmap[i]) - 1;
    }
  }

  return -1;  // No set bit found
}

/**
 * Utility function to split a block into smaller blocks (creates buddies)
 *
 * @param order The order of the block to split
 * @param bit The bit of the block to split
 */
static void split_block(int order, int bit) {
  for (int i = order; i > 0; i--) {
    int buddy_bit = bit ^ (1 << (i - 1));
    set_bit(bitmap[i - 1], buddy_bit);
  }
}

/**
 * Utility function to marka a block as used
 *
 * @param order The order of the block to mark as used
 * @param bit The bit of the block to mark as used
 */
static void mark_block_used(int order, int bit) {
  clear_bit(bitmap[order], bit);
}

/**
 * Utility function to find which order a given size will fit into
 *
 * @param size The size to find the order for
 * @return The order that the size fits into
 */
static int get_order(size_t size) {
  int order = 0;
  size = (size - 1) >> PAGE_SHIFT;

  while (size > 0) {
    order++;
    size >>= 1;
  }

  return order;
}

/**
 * Utility function to find the first usable free block in a given order
 *
 * @param order The order of the block to find
 * @return The bit of the first free block, or -1 if no free block is found
 */
static int find_free_block(int order) {
  for (int i = order; i <= MAX_ORDER; i++) {
    int bit = find_first_set(bitmap[i], 1 + (1 << MAX_ORDER) / 64);

    if (bit != -1) {
      return bit;
    }
  }

  return -1;
}

/**
 * Helper function to get a human-readable name for a memory map entry type
 *
 * @param type Memory map entry type
 * @return Human-readable name for the type
 */
static const char* get_memmap_type_string(uint64_t type) {
  if (type < MEMMAP_TYPE_COUNT) {
    return memmap_type_strings[type];
  }

  return "Unknown";
}

/**
 * Utility function that formats a memory size as a human-readable string
 *
 * @param size Size in bytes
 * @param buffer Buffer to write the formatted string to
 * @param buffer_size Size of the buffer
 * @return Pointer to the buffer
 */
static char* format_memory_size(uint64_t size, char* buffer, size_t buffer_size) {
  uint64_t kib = size / 1024;
  uint64_t mib = kib / 1024;
  uint64_t gib = mib / 1024;

  if (gib > 0) {
    uint64_t mib_remainder = mib % 1024;
    snprintf_(buffer, buffer_size, "%llu bytes (%llu GiB + %llu MiB)", size, gib, mib_remainder);
  } else if (mib > 0) {
    uint64_t kib_remainder = kib % 1024;
    snprintf_(buffer, buffer_size, "%llu bytes (%llu MiB + %llu KiB)", size, mib, kib_remainder);
  } else {
    snprintf_(buffer, buffer_size, "%llu bytes (%llu KiB)", size, kib);
  }

  return buffer;
}

void pmm_initialize(
  uint64_t entry_count,
  struct limine_memmap_entry **entries,
  struct limine_kernel_address_response *kernel_address_response,
  struct limine_kernel_file_response *kernel_file_response
) {
  total_memory = 0;
  usable_memory = 0;
  char size_buffer[64];

  // Calculate kernel_end using the file size from kernel_file_response
  kernel_start = kernel_address_response->virtual_base;
  uint64_t kernel_size = kernel_file_response->kernel_file->size;
  kernel_end = kernel_start + kernel_size;

  // Round up kernel_end to the next page boundary
  kernel_end = (kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "  Reading memory map\n",
      format_memory_size(total_memory, size_buffer, sizeof(size_buffer))
    );
  }

  // Process each entry
  for (size_t i = 0; i < entry_count; i++) {
    // entry->base is the base address of this memory region
    // entry->length is the length of this region
    // entry->type describes the type of this memory region
    struct limine_memmap_entry *entry = entries[i];

    if (DEBUG) {
      const char* entry_type = get_memmap_type_string(entry->type);

      log_message(
        &kernel_debug_logger,
        LOG_DEBUG,
        "    {base=0x%016llx, length=%s, type=%s}\n",
        entry->base,
        format_memory_size(entry->length, size_buffer, sizeof(size_buffer)),
        entry_type
      );
    }

    total_memory += entry->length;

    // Sum up usable memory
    if (entry->type == LIMINE_MEMMAP_USABLE) {
      usable_memory += entry->length;
    }
  }

  // Calculate total frames
  total_frames = total_memory / PAGE_SIZE;

  // The bitmap needs to be initialized to all 1s (all memory is free)
  memset(bitmap, 0xFF, sizeof(bitmap));

  // Memory mapping:

  // 1. When the buddy allocator is initialized, it starts by viewing the entire manageable memory space as being composed of the largest possible free blocks (highest order).

  // 2. In a real OS, you'd typically have a memory map (often provided by the bootloader) that tells you which regions of physical memory are usable.
  // The physical memory manager would use this map to initialize the buddy allocator, marking unusable regions (like those reserved for hardware) as allocated from the start.

  // 3. During initialization, the allocator marks all the highest-order blocks that fit within the physical memory as free.
  // Any remaining memory that's smaller than the highest-order block size is handled as the next lower order, and so on, until all memory is accounted for.

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "  Total system memory: %s\n",
      format_memory_size(total_memory, size_buffer, sizeof(size_buffer))
    );

    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "  Usable system memory: %s\n",
      format_memory_size(usable_memory, size_buffer, sizeof(size_buffer))
    );

    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "  Kernel address: %p\n",
      kernel_start
    );

    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "  Kernel size: %s\n",
      format_memory_size(kernel_size, size_buffer, sizeof(size_buffer))
    );
  }
}

uintptr_t pmm_alloc(size_t size) {
  int order = get_order(size);
  int bit = find_free_block(order);

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_DEBUG,
      "PMM: Requested %d bytes\n",
      size
    );
  }

  if (bit == -1) {
    // No free block available
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
      LOG_ERROR,
      "  Free block found at bit %d for order: %d\n",
      bit,
      order
    );
  }

  // Split blocks if necessary
  if (order < MAX_ORDER) {
    split_block(order, bit);
  }

  // Mark the block as used
  mark_block_used(order, bit);

  // Calculate the physical address
  uintptr_t address = bit << (PAGE_SHIFT + order);

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_DEBUG,
      "  Allocated %d bytes (requested: %d bytes) at 0x%016lx\n",
      round_to_nearest_pow2(size),
      size,
      (void*)address
    );
  }

  return address;
}

void pmm_free(uintptr_t addr, size_t size) {
  size_t rounded_size = round_to_nearest_pow2(size);

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_DEBUG,
      "PMM: Freeing %d bytes at %p (requested: %d bytes)\n",
      rounded_size,
      addr,
      size
    );
  }

}

size_t pmm_get_total_memory() {

}

size_t pmm_get_free_memory() {

}

void pmm_debug_print_state() {
  char buffer[64];

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
      uint64_t current_word = bitmap[order][word];

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
