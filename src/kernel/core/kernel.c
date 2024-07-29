#define SSFN_CONSOLEBITMAP_TRUECOLOR

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>
#include <ssfn.h>

#include <kernel/bootloader.h>
#include <kernel/string.h>
#include <kernel/terminal.h>
#include <kernel/serial.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/debug_logger.h>

struct limine_file *getFile(const char *name) {
  struct limine_module_response *module_response = module_request.response;

  for (size_t i = 0; i < module_response->module_count; i++) {
    struct limine_file *f = module_response->modules[i];

    if (checkStringEndsWith(f->path, name)) {
      return f;
    }
  }

  return NULL;
}

// Halt and catch fire function.
static void hcf(void) {
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

/**
 * Kernel entry point
 */
void _start(void) {
  serial_initialize();
  debug_logger_initialize();

  log_message(&kernel_debug_logger, LOG_INFO, "Mirus kernel intialization\n\n.");
  log_message(&kernel_debug_logger, LOG_INFO, "Serial driver loaded\n");

  // Ensure the bootloader actually understands our base revision (see spec).
  log_message(&kernel_debug_logger, LOG_INFO, "Checking bootloader version compatability...");
  if (LIMINE_BASE_REVISION_SUPPORTED == false) {
    log_message(&kernel_debug_logger, LOG_INFO, "failed\n");
    hcf();
  }
  log_message(&kernel_debug_logger, LOG_INFO, "done\n");

  // Ensure we got a framebuffer and fetch the first available one.
  log_message(&kernel_debug_logger, LOG_INFO, "Getting framebuffer...");
  if (framebuffer_request.response == NULL
    || framebuffer_request.response->framebuffer_count < 1) {
    log_message(&kernel_debug_logger, LOG_INFO, "failed\n");
    hcf();
  }
  struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
  log_message(&kernel_debug_logger, LOG_INFO, "done\n");

  // load font
  log_message(&kernel_debug_logger, LOG_INFO, "Loading default font...");
  struct limine_file *default_terminal_font = getFile("u_vga16.sfn");
  if (default_terminal_font == NULL) {
    log_message(&kernel_debug_logger, LOG_INFO, "failed\n");
    hcf();
  }
  log_message(&kernel_debug_logger, LOG_INFO, "done\n");

  log_message(&kernel_debug_logger, LOG_INFO, "Initializing terminal...");
  terminal_initialize(default_terminal_font, framebuffer);
  log_message(&kernel_debug_logger, LOG_INFO, "done\n");

  terminal_write_string("Mirus!\n\n");

  log_message(&kernel_debug_logger, LOG_INFO, "Initializing GDT\n");
  gdt_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Initializing GDT...done\n");

  log_message(&kernel_debug_logger, LOG_INFO, "Initializing IDT\n");
  idt_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Initializing IDT...done\n");

  // terminal_write_string("Initializing LDT...");
  // serial_write_string("Initializing LDT...");
  // // TODO: do the LDT stuff
  // terminal_write_string("done\n");
  // serial_write_string("done\n");

  // terminal_write_string("Initializing ISRs...");
  // serial_write_string("Initializing ISRs...");
  // // TODO: do the ISR stuff
  // terminal_write_string("done\n");
  // serial_write_string("done\n");

  // terminal_write_string("Initializing IRQs...");
  // serial_write_string("Initializing IRQs...");
  // // TODO: do the IRQ stuff
  // terminal_write_string("done\n");
  // serial_write_string("done\n");

  // We're done, just hang...
  // terminal_write_string("\nKernel initialization complete.\n");
  // serial_write_string("\nKernel initialization complete.\n");
  hcf();
}
