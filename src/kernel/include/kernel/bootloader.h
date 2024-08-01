#pragma once

#include <stdint.h>

#include <limine/limine.h>
#include <libk/string.h>

/**
 * Get bootloader version from Limine
 */
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

/**
 * Get the framebuffer from Limine
 */
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
  .id = LIMINE_FRAMEBUFFER_REQUEST,
  .revision = 0
};

/**
 * Get kernel modules from Limine
 */
__attribute__((used, section(".requests")))
static volatile struct limine_module_request module_request = {
  .id = LIMINE_MODULE_REQUEST,
  .revision = 0
};

/**
 * Get memory map from Limine
 */
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memory_map_request = {
  .id = LIMINE_MEMMAP_REQUEST,
  .revision = 0
};

/**
 * Get stack size from Limine
 *
 * TODO: We won't need this once we have a proper stack (depends on memory
 *       manager)
 */
__attribute__((used, section(".requests")))
static volatile struct limine_stack_size_request stack_size_request = {
  .id = LIMINE_STACK_SIZE_REQUEST,
  .revision = 0,
  .stack_size = 16384 // 16 KiB
};

/**
 * Get kernel address from Limine
 *
 * TODO: We won't need this once we have a proper stack (depends on memory
 *       manager)
 */
__attribute__((used, section(".requests")))
static volatile struct limine_kernel_address_request kernel_address_request = {
  .id = LIMINE_KERNEL_ADDRESS_REQUEST,
  .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_kernel_file_request kernel_file_request = {
    .id = LIMINE_KERNEL_FILE_REQUEST,
    .revision = 0
};





__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

/**
 * Get a file from a bundled kernel module
 */
static inline struct limine_file *limine_get_file(const char *name) {
  struct limine_module_response *module_response = module_request.response;

  for (size_t i = 0; i < module_response->module_count; i++) {
    struct limine_file *f = module_response->modules[i];

    if (checkStringEndsWith(f->path, name)) {
      return f;
    }
  }

  return NULL;
}
