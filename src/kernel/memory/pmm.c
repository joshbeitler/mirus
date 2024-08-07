#include <stdint.h>
#include <stddef.h>

#include <printf/printf.h>
#include <limine/limine.h>
#include <jemi/jemi.h>

#include <hal/serial.h>

#include <kernel/paging.h>
#include <kernel/debug.h>
#include <kernel/pmm.h>
#include <kernel/buddy.h>

#define JEMI_NODES_MAX 1000  // Adjust this based on expected maximum number of memory map entries

// TODO: Finish memory size functions

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

/**
 * Internal helper variables
 */
static uintptr_t kernel_start, kernel_end;
static uint64_t total_memory, usable_memory;
static uint64_t total_frames;

static BuddyAllocator buddy_allocator;

// defines a series of buddy allocators to cover the entire system memory
// #define BUDDY_ALLOCATOR_COUNT 1 // TODO: should be calculated based on system memory
// static BuddyAllocator buddy_allocators[BUDDY_ALLOCATOR_COUNT];

/**
 * Helper function to get a human-readable name for a memory map entry type
 *
 * @param type Memory map entry type
 *
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
 *
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

/** testing the jemi log emitter */
static void jemi_writer(char ch, void *arg) {
  char **buf = (char **)arg;
  **buf = ch;
  (*buf)++;
}

static void log_memory_map_debug(
  uint64_t entry_count,
  struct limine_memmap_entry **entries,
  uint64_t total_memory,
  uint64_t usable_memory,
  uintptr_t kernel_start,
  uint64_t kernel_size
) {
  static jemi_node_t node_pool[JEMI_NODES_MAX];
  jemi_init(node_pool, JEMI_NODES_MAX);

  // Create the entries array first
  jemi_node_t *entries_array = jemi_array(NULL);

  // Process each entry
  for (size_t i = 0; i < entry_count; i++) {
    struct limine_memmap_entry *entry = entries[i];

    // Create a JSON object for this entry
    jemi_node_t *entry_obj = jemi_object(
      jemi_string("base"), jemi_integer(entry->base),
      jemi_string("length"), jemi_integer(entry->length),
      jemi_string("type"), jemi_string(get_memmap_type_string(entry->type)),
      NULL
    );

    // Add the entry to the entries array
    jemi_array_append(entries_array, entry_obj);
  }

  // Create the root object for the memory map
  jemi_node_t *root = jemi_object(
    jemi_string("total_memory"), jemi_integer(total_memory),
    jemi_string("usable_memory"), jemi_integer(usable_memory),
    jemi_string("kernel_start"), jemi_integer(kernel_start),
    jemi_string("kernel_size"), jemi_integer(kernel_size),
    jemi_string("entries"), entries_array,
    NULL
  );

  // Convert the jemi structure to a JSON string
  char json_str[JEMI_NODES_MAX * 64];  // Adjust size as needed
  char *json_ptr = json_str;
  jemi_emit(root, jemi_writer, &json_ptr);
  *json_ptr = '\0';  // Null-terminate the string

  // Log the complex data
  log_complex(&kernel_debug_logger, LOG_INFO, "memory_manager", "Memory Map", json_str);

  // Reset jemi for future use
  jemi_reset();
}
/** end testing the jemi log emitter */

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
      "memory_manager",
      "Reading memory map\n",
      format_memory_size(total_memory, size_buffer, sizeof(size_buffer))
    );
  }

  // Process each entry
  for (size_t i = 0; i < entry_count; i++) {
    // entry->base is the base address of this memory region
    // entry->length is the length of this region
    // entry->type describes the type of this memory region
    struct limine_memmap_entry *entry = entries[i];

    // if (DEBUG) {
    //   const char* entry_type = get_memmap_type_string(entry->type);

    //   log_message(
    //     &kernel_debug_logger,
    //     LOG_DEBUG,
    //     "memory_manager",
    //     "    {base=0x%016llx, length=%s, type=%s}\n",
    //     entry->base,
    //     format_memory_size(entry->length, size_buffer, sizeof(size_buffer)),
    //     entry_type
    //   );
    // }

    total_memory += entry->length;

    // Sum up usable memory
    if (entry->type == LIMINE_MEMMAP_USABLE) {
      usable_memory += entry->length;
    }
  }

  if (DEBUG) {
    log_memory_map_debug(entry_count, entries, total_memory, usable_memory, kernel_start, kernel_size);
  }

  // Calculate total frames
  total_frames = total_memory / PAGE_SIZE;

  // The bitmap needs to be initialized to all 1s (all memory is free)
  buddy_allocator_init(&buddy_allocator, 0); // TODO: acutally initialize with address

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
      "memory_manager",
      "Total system memory: %s\n",
      format_memory_size(total_memory, size_buffer, sizeof(size_buffer))
    );

    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "memory_manager",
      "Usable system memory: %s\n",
      format_memory_size(usable_memory, size_buffer, sizeof(size_buffer))
    );

    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "memory_manager",
      "Kernel address: %p\n",
      kernel_start
    );

    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "memory_manager",
      "Kernel size: %s\n",
      format_memory_size(kernel_size, size_buffer, sizeof(size_buffer))
    );
  }
}

// TODO: does this get replaced with kmalloc?
uintptr_t pmm_alloc(size_t size) {
  return buddy_allocator_alloc(&buddy_allocator, size);
}

// TODO: does this get replaced with kfree?
void pmm_free(uintptr_t address, size_t size) {
  buddy_allocator_free(&buddy_allocator, address, size);
}

size_t pmm_get_total_memory() {

}

size_t pmm_get_free_memory() {

}

void pmm_debug_print_state() {
  buddy_allocator_dump_bitmap(&buddy_allocator);
}
