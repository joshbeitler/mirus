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
  serial_write_string("Mirus kernel initialization\n\n");
  serial_write_string("Setting up serial driver...done\n");
  serial_write_string("Checking bootloader compatability...");

  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED == false) {
    serial_write_string("failed\n");
    hcf();
  }

  serial_write_string("done\n");

  serial_write_string("Getting framebuffer...");

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL
    || framebuffer_request.response->framebuffer_count < 1) {
    serial_write_string("failed\n");
    hcf();
  }

  // Fetch the first framebuffer.
  struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
  serial_write_string("done\n");

  // load font
  serial_write_string("Getting default terminal font...");
  struct limine_file *default_terminal_font = getFile("u_vga16.sfn");

  if (default_terminal_font == NULL) {
    serial_write_string("failed\n");
    hcf();
  }

  serial_write_string("done\n");

  serial_write_string("Initializing terminal...");
  terminal_initialize(default_terminal_font, framebuffer);
  serial_write_string("done\n");
  terminal_write_string("Mirus!\n\n");
  terminal_write_string("Framebuffer, font renderer, and terminal...ok\n");

  terminal_write_string("Initializing GDT...");
  serial_write_string("Initializing GDT...\n");
  // TODO: do the GDT stuff
  gdt_initialize();
  terminal_write_string("done\n");
  serial_write_string("done\n");

  terminal_write_string("Initializing LDT...");
  serial_write_string("Initializing LDT...");
  // TODO: do the LDT stuff
  terminal_write_string("done\n");
  serial_write_string("done\n");

  terminal_write_string("Initializing IDT...");
  serial_write_string("Initializing IDT...");
  // TODO: do the IDT stuff
  terminal_write_string("done\n");
  serial_write_string("done\n");

  terminal_write_string("Initializing ISRs...");
  serial_write_string("Initializing ISRs...");
  // TODO: do the ISR stuff
  terminal_write_string("done\n");
  serial_write_string("done\n");

  terminal_write_string("Initializing IRQs...");
  serial_write_string("Initializing IRQs...");
  // TODO: do the IRQ stuff
  terminal_write_string("done\n");
  serial_write_string("done\n");

  // We're done, just hang...
  terminal_write_string("\nKernel initialization complete.\n");
  serial_write_string("\nKernel initialization complete.\n");
  hcf();
}
