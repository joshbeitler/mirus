#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <printf/printf.h>
#include <limine/limine.h>

#include <kernel/pmm.h>
#include <kernel/paging.h>
#include <kernel/debug.h>


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
static uint64_t bitmap[MAX_ORDER][1 + (1 << MAX_ORDER) / 64];

static void set_bit(uint64_t *bitmap, int bit) {
  bitmap[bit / 64] |= (1ULL << (bit % 64));
}

static void clear_bit(uint64_t *bitmap, int bit) {
  bitmap[bit / 64] &= ~(1ULL << (bit % 64));
}

static int test_bit(uint64_t *bitmap, int bit) {
  return (bitmap[bit / 64] & (1ULL << (bit % 64))) != 0;
}

// Find first set bit in the bitmap
static int find_first_set(uint64_t *bitmap, int size) {
  for (int i = 0; i < size; i++) {
    if (bitmap[i] != 0) {
      return i * 64 + __builtin_ffs(bitmap[i]);
    }
  }
  return -1;  // No set bit found
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

char* format_memory_size(uint64_t size, char* buffer, size_t buffer_size) {
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
        "  {base=0x%016llx, length=%s, type=%s}\n",
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

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "Total system memory: %s\n",
      format_memory_size(total_memory, size_buffer, sizeof(size_buffer))
    );

    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "Usable system memory: %s\n",
      format_memory_size(usable_memory, size_buffer, sizeof(size_buffer))
    );

    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "Kernel address: %p\n",
      kernel_start
    );

    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "Kernel size: %s\n",
      format_memory_size(kernel_size, size_buffer, sizeof(size_buffer))
    );
  }
}

uintptr_t pmm_alloc(size_t size) {

}

void pmm_free(uintptr_t addr, size_t size) {

}

size_t pmm_get_total_memory() {

}

size_t pmm_get_free_memory() {

}

void pmm_debug_print_state() {

}
