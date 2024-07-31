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

#include <kernel/bootloader.h>
#include <kernel/interrupts.h>
#include <kernel/memory.h>
#include <kernel/debug.h>
#include <kernel/panic.h>
#include <kernel/syscalls.h>

#include <drivers/terminal.h>

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
  read_memory_map(memory_map_request.response->entry_count, memory_map_request.response->entries);

  log_message(&kernel_debug_logger, LOG_INFO, "Starting system call initialization\n");
  syscalls_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized system calls\n");

  log_message(&kernel_debug_logger, LOG_INFO, "Kernel initialization complete\n");
  printf_("Mirus, ahoy!\n\n");


  debug_test_exceptions();
  // debug_test_syscalls();

  // We're done, just hang...
  hcf();
}
