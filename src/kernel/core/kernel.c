#define SSFN_CONSOLEBITMAP_TRUECOLOR

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine/limine.h>
#include <ssfn/ssfn.h>
#include <printf/printf.h>

#include <hal/serial.h>
#include <hal/gdt.h>
#include <hal/idt.h>
#include <hal/hal_logger.h>

#include <kernel/terminal.h>
#include <kernel/boot/requests.h>
#include <kernel/interrupt/isr.h>
#include <kernel/debug/debug_logger.h>
#include <kernel/debug/panic.h>

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

/**
 * Kernel entry point
 */
void _start(void) {
  serial_initialize();
  debug_logger_initialize();
  hal_logger_initialize();

  log_message(&kernel_debug_logger, LOG_INFO, "Mirus kernel intialization\n");
  log_message(&kernel_debug_logger, LOG_INFO, "Serial driver loaded\n");

  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED == false) {
    log_message(&kernel_debug_logger, LOG_FATAL, "Bootloader version not compatible\n");
    hcf();
  }
  log_message(&kernel_debug_logger, LOG_INFO, "Bootloader version compatible\n");

  // Ensure we got a framebuffer and fetch the first available one.
  if (framebuffer_request.response == NULL
    || framebuffer_request.response->framebuffer_count < 1) {
    log_message(&kernel_debug_logger, LOG_FATAL, "Couldn't get framebuffer\n");
    hcf();
  }
  struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully loaded framebuffer\n");

  // Ensure we got a memory map
  if (memory_map_request.response == NULL
    || memory_map_request.response->entry_count == 0) {
    log_message(&kernel_debug_logger, LOG_FATAL, "Couldn't get memory map\n");
    hcf();
  }
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully loaded memory map\n");

  // load font
  struct limine_file *default_terminal_font = limine_get_file("u_vga16.sfn");
  if (default_terminal_font == NULL) {
    log_message(&kernel_debug_logger, LOG_FATAL, "Couldn't load default font: u_vga16.sfn\n");
    hcf();
  }
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully loaded default font: u_vga16.sfn\n");

  terminal_initialize(default_terminal_font, framebuffer);
  log_message(&kernel_debug_logger, LOG_INFO, "Virtual terminal initialized\n");

  log_message(&kernel_debug_logger, LOG_INFO, "Starting GDT initialization\n");
  gdt_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized GDT\n");

  log_message(&kernel_debug_logger, LOG_INFO, "Starting IDT initialization\n");
  idt_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized IDT\n");

  log_message(&kernel_debug_logger, LOG_INFO, "Starting ISR initialization\n");
  isr_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized ISRs\n");

  // Look at memory map
  // do somthing with the entries. we will move this somewhere else
  log_message(&kernel_debug_logger, LOG_INFO, "Reading memory map\n");
  uint64_t total_memory = 0;
  uint64_t usable_memory = 0;
  char size_buffer[64];
  for (size_t i = 0; i < memory_map_request.response->entry_count; i++) {
    struct limine_memmap_entry *entry = memory_map_request.response->entries[i];

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

  log_message(&kernel_debug_logger, LOG_INFO, "Kernel initialization complete\n");

  printf_("Mirus, ahoy!\n\n");

  // log_message(&kernel_debug_logger, LOG_INFO, "Trying exception handler\n");

  // try the exception handler
  // int a = 10;
  // int b = 0;
  // int c = a / b; // This will generate a Division By Zero exception
  // // This line may not be reached; depends on how your handler reacts (e.g., halt system, log and continue)
  // log_message(&kernel_debug_logger, LOG_INFO, "Result of division: %d\n", c);

  // printf_("We're still here?\n");

  // terminal_write_string("Initializing LDT...");
  // serial_write_string("Initializing LDT...");
  // // TODO: do the LDT stuff
  // terminal_write_string("done\n");
  // serial_write_string("done\n");

  // terminal_write_string("Initializing IRQs...");
  // serial_write_string("Initializing IRQs...");
  // // TODO: do the IRQ stuff
  // terminal_write_string("done\n");
  // serial_write_string("done\n");

  // We're done, just hang...
  hcf();
}
