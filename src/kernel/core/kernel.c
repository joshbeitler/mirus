#define SSFN_CONSOLEBITMAP_TRUECOLOR

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>
#include <ssfn.h>

#include <bootloader.h>
#include <string.h>
#include <terminal.h>

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
  asm ("cli");
  for (;;) {
    asm ("hlt");
  }
}

/**
 * Kernel entry point
 */
void _start(void) {
  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED == false) {
    hcf();
  }

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL
    || framebuffer_request.response->framebuffer_count < 1) {
    hcf();
  }

  // Fetch the first framebuffer.
  struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

  // load font
  struct limine_file *default_terminal_font = getFile("u_vga16.sfn");

  if (default_terminal_font == NULL) {
    hcf();
  }

  terminal_initialize(default_terminal_font, framebuffer);
  terminal_write_string("Hello, World!");

  // We're done, just hang...
  hcf();
}
