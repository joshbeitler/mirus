#include <stdint.h>
#include <stddef.h>

#include <printf/printf.h>
#include <limine/limine.h>

#include <kernel/memory.h>
#include <kernel/debug.h>

const char* get_memmap_type_string(uint64_t type) {
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

  for (size_t i = 0; i < entry_count; i++) {
    // Process each entry
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
