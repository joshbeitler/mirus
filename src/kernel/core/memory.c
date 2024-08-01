#include <stdint.h>
#include <stddef.h>

#include <printf/printf.h>
#include <limine/limine.h>

#include <kernel/memory.h>
#include <kernel/debug.h>

// uint64_t total_memory = 0;
// uint64_t usable_memory = 0;

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

void read_memory_map(uint64_t entry_count, struct limine_memmap_entry **entries, struct limine_kernel_address_response *kernel_address_response, struct limine_kernel_file_response *kernel_file_response) {
  total_memory = 0;
  usable_memory = 0;
  char size_buffer[64];

  if (kernel_address_response == NULL) {
          // Handle error: Limine didn't provide kernel address info
          return;
      }
  kernel_start = kernel_address_response->virtual_base;

  // // Calculate kernel_end using the file size from kernel_file_request
  uint64_t kernel_size = kernel_file_response->kernel_file->size;
  // uint64_t kernel_size = 10 * 1024 * 1024;
  kernel_end = kernel_start + kernel_size;

  // // Round up kernel_end to the next page boundary
  kernel_end = (kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);



  for (size_t i = 0; i < entry_count; i++) {
    struct limine_memmap_entry *entry = entries[i];

    // Process each entry
    // entry->base is the base address of this memory region
    // entry->length is the length of this region
    // entry->type describes the type of this memory region

    const char* entry_type = get_memmap_type_string(entry->type);
    log_message(
      &kernel_debug_logger,
      LOG_DEBUG,
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


// siiimple bitmap page allocator


// #define PAGE_SIZE 4096
// #define PAGE_PRESENT (1 << 0)
// #define PAGE_WRITABLE (1 << 1)
// #define PAGES_PER_WORD 64  // Number of pages represented by each bitmap word

// static uintptr_t kernel_start;
// static uintptr_t kernel_end;

// static uint64_t *page_bitmap;  // Pointer to our bitmap
// static uint64_t total_pages;   // Total number of pages in the system
// static uint64_t free_pages;    // Number of free pages

uint64_t* get_pml4_address(void) {
    uint64_t pml4;
    asm volatile ("mov %%cr3, %0" : "=r" (pml4));
    return (uint64_t*)pml4;
}

void invlpg(uint64_t virtual_address) {
    asm volatile ("invlpg (%0)" :: "r" (virtual_address) : "memory");
}

void map_page(uint64_t virtual_address, uint64_t physical_address, uint64_t flags) {
    uint64_t pml4_entry, pdpt_entry, pd_entry, pt_entry;
    uint64_t *pml4, *pdpt, *pd, *pt;

    // Get the page table indices
    pml4_entry = (virtual_address >> 39) & 0x1FF;
    pdpt_entry = (virtual_address >> 30) & 0x1FF;
    pd_entry = (virtual_address >> 21) & 0x1FF;
    pt_entry = (virtual_address >> 12) & 0x1FF;

    // Get the PML4 table
    pml4 = get_pml4_address();

    // Check if PDPT exists, if not create it
    if (!(pml4[pml4_entry] & PAGE_PRESENT)) {
        uint64_t pdpt_physical = allocate_page(); // You need to implement this function
        pml4[pml4_entry] = pdpt_physical | PAGE_PRESENT | PAGE_WRITABLE | flags;
    }
    pdpt = (uint64_t*)((pml4[pml4_entry] & ~0xFFF) + kernel_start);

    // Check if PD exists, if not create it
    if (!(pdpt[pdpt_entry] & PAGE_PRESENT)) {
        uint64_t pd_physical = allocate_page(); // You need to implement this function
        pdpt[pdpt_entry] = pd_physical | PAGE_PRESENT | PAGE_WRITABLE | flags;
    }
    pd = (uint64_t*)((pdpt[pdpt_entry] & ~0xFFF) + kernel_start);

    // Check if PT exists, if not create it
    if (!(pd[pd_entry] & PAGE_PRESENT)) {
        uint64_t pt_physical = allocate_page(); // You need to implement this function
        pd[pd_entry] = pt_physical | PAGE_PRESENT | PAGE_WRITABLE | flags;
    }
    pt = (uint64_t*)((pd[pd_entry] & ~0xFFF) + kernel_start);

    // Map the page
    pt[pt_entry] = (physical_address & ~0xFFF) | PAGE_PRESENT | flags;

    // Invalidate the TLB entry for this page
    invlpg(virtual_address);
}

void init_page_allocator() {
    total_pages = usable_memory / PAGE_SIZE;
    free_pages = total_pages;

    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "Total memory pages: %d\n",
      total_pages
    );
    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "Free memory pages: %d\n",
      free_pages
    );

    // Calculate the size of the bitmap in bytes
    uint64_t bitmap_size = (total_pages + PAGES_PER_WORD - 1) / PAGES_PER_WORD * sizeof(uint64_t);

    // Place the bitmap just after the kernel
    page_bitmap = (uint64_t*)(((uintptr_t)&kernel_end + PAGE_SIZE - 1) & ~((uintptr_t)PAGE_SIZE - 1));

    // Mark all pages as free (0 in the bitmap means free)
    for (uint64_t i = 0; i < bitmap_size / sizeof(uint64_t); i++) {
        page_bitmap[i] = 0;
    }

    // Mark pages used by the kernel and the bitmap itself as used
    uint64_t kernel_pages = ((uint64_t)page_bitmap + bitmap_size - (uint64_t)&kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint64_t i = 0; i < kernel_pages; i++) {
        uint64_t idx = i / PAGES_PER_WORD;
        uint64_t bit = i % PAGES_PER_WORD;
        page_bitmap[idx] |= (1ULL << bit);
        free_pages--;
    }
}

uintptr_t allocate_page(void) {
    if (free_pages == 0) {
        return 0;  // Out of memory
    }
    for (uint64_t i = 0; i < total_pages / PAGES_PER_WORD; i++) {
        if (page_bitmap[i] != ~0ULL) {  // This word has at least one free page
            for (int j = 0; j < PAGES_PER_WORD; j++) {
                if ((page_bitmap[i] & (1ULL << j)) == 0) {
                    uintptr_t page_addr = (i * PAGES_PER_WORD + j) * PAGE_SIZE;
                    // Ensure we're not allocating memory below kernel_end
                    if (page_addr < kernel_end) {
                        continue;  // Skip this page and try the next one
                    }
                    page_bitmap[i] |= (1ULL << j);  // Mark the page as used
                    free_pages--;
                    return page_addr;
                }
            }
        }
    }
    return 0;  // Should never reach here if free_pages > 0
}



void free_page(uintptr_t page_addr) {
    uint64_t page_index = page_addr / PAGE_SIZE;
    uint64_t word_index = page_index / PAGES_PER_WORD;
    uint64_t bit_index = page_index % PAGES_PER_WORD;

    // Check if the page is already free
    if ((page_bitmap[word_index] & (1ULL << bit_index)) == 0) {
        // Page is already free, handle error (maybe panic)
        return;
    }

    // Mark the page as free
    page_bitmap[word_index] &= ~(1ULL << bit_index);
    free_pages++;
}
