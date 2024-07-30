#include <stdint.h>
#include <stddef.h>

#include <printf/printf.h>
#include <limine/limine.h>

#include <kernel/memory/memory.h>
#include <kernel/debug/debug_logger.h>

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

void read_memory_map(uint64_t entry_count, struct limine_memmap_entry **entries) {
  uint64_t total_memory = 0;
  uint64_t usable_memory = 0;
  char size_buffer[64];
  for (size_t i = 0; i < entry_count; i++) {
    struct limine_memmap_entry *entry = entries[i];

    // Process each entry
    // entry->base is the base address of this memory region
    // entry->length is the length of this region
    // entry->type describes the type of this memory region

    // Example: Print out memory map information
    // Note: You'll need to implement your own print function
    const char* entry_type = get_memmap_type_string(entry->type);
    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "  {base=0x%016llx, length=%s, type=%s}\n",
      entry->base,
      format_memory_size(entry->length, size_buffer, sizeof(size_buffer)),
      entry_type
    );

    total_memory += entry->length;

    // Sum up usable memory
    if (entry->type == LIMINE_MEMMAP_USABLE) {
      usable_memory += entry->length;
    }
  }

  log_message(
    &kernel_debug_logger,
    LOG_INFO,
    "Total system memory %s\n",
    format_memory_size(total_memory, size_buffer, sizeof(size_buffer))
  );
  log_message(
    &kernel_debug_logger,
    LOG_INFO,
    "Usable system memory %s\n",
    format_memory_size(usable_memory, size_buffer, sizeof(size_buffer))
  );
}
