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

#include <libk/string.h>

#include <kernel/bootloader.h>
#include <kernel/terminal.h>
#include <kernel/interrupt/isr.h>
#include <kernel/debug/debug_logger.h>
#include <kernel/debug/panic.h>

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

  // load font
  struct limine_file *default_terminal_font = getFile("u_vga16.sfn");
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
