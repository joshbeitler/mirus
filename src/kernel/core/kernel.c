#define SSFN_CONSOLEBITMAP_TRUECOLOR

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine/limine.h>
#include <ssfn/ssfn.h>
#include <printf/printf.h>
#include <libk/string.h>

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
#include <kernel/stack.h>
#include <kernel/process.h>

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

  // Find kernel stack from bootloader
  uintptr_t kernel_stack_top = get_kernel_stack_ptr();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully found kernel stack at {top=%p}\n", (void*)kernel_stack_top);

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

  // Ensure we got the kernel location
  if (kernel_address_request.response == NULL
    || kernel_address_request.response->virtual_base == 0) {
    log_message(&kernel_debug_logger, LOG_FATAL, "Couldn't get kernel location\n");
    hcf();
  }
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully found kernel location\n");

  // Ensure we got the kernel file
  if (kernel_file_request.response == NULL
    || kernel_file_request.response->kernel_file == NULL) {
    log_message(&kernel_debug_logger, LOG_FATAL, "Couldn't get kernel file\n");
    hcf();
  }
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully found kernel file\n");

  // Load default bitmap font
  struct limine_file *default_terminal_font = limine_get_file("u_vga16.sfn");
  if (default_terminal_font == NULL) {
    log_message(&kernel_debug_logger, LOG_FATAL, "Couldn't load default font: {file=u_vga16.sfn}\n");
    hcf();
  }
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully loaded default font: {file=u_vga16.sfn}\n");

  // Set up terminal
  terminal_initialize(default_terminal_font, framebuffer);
  log_message(&kernel_debug_logger, LOG_INFO, "Virtual terminal initialized\n");

  // Set up the GDT and TSS
  log_message(&kernel_debug_logger, LOG_INFO, "Starting GDT initialization\n");
  gdt_initialize(kernel_stack_top);
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized GDT\n");

  // Set up the IDT
  log_message(&kernel_debug_logger, LOG_INFO, "Starting IDT initialization\n");
  idt_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized IDT\n");

  // Load the IDT with handlers
  log_message(&kernel_debug_logger, LOG_INFO, "Starting ISR initialization\n");
  isr_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized ISRs\n");

  // Read the memory map and initialize the physical memory manager with it
  log_message(&kernel_debug_logger, LOG_INFO, "Starting physical memory manager initialization\n");
  pmm_initialize(
    memory_map_request.response->entry_count,
    memory_map_request.response->entries,
    kernel_address_request.response,
    kernel_file_request.response
  );
  // TODO: Initialize paging
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized physical memory manager\n");

  // Set up system calls
  log_message(&kernel_debug_logger, LOG_INFO, "Starting system call initialization\n");
  syscalls_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized system calls\n");

  // We're done! Let the user know
  // TODO: This will eventually be replaced with a userspace jump to the
  //       init process.
  log_message(&kernel_debug_logger, LOG_INFO, "Kernel initialization complete\n");
  printf_("Mirus, ahoy!\n\n");

  // If we got here, just chill. Halt the CPU.
  hcf();
}
