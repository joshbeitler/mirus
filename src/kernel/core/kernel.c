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

  // Note: we assume the framebuffer model is RGB with 32-bit pixels.
  // for (size_t i = 0; i < 100; i++) {
  //   volatile uint32_t *fb_ptr = framebuffer->address;
  //   fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
  // }

  // load font
  const char *fName = "u_vga16.sfn";
  struct limine_file *file = getFile(fName);

  if (file == NULL) {
    hcf();
  }

  ssfn_src = file->address;

  ssfn_dst.ptr = framebuffer->address;
  ssfn_dst.w = framebuffer->width;
  ssfn_dst.h = framebuffer->height;
  ssfn_dst.p = framebuffer->pitch;
  ssfn_dst.x = ssfn_dst.y = 0;
  ssfn_dst.fg = 0xFFFFFF;

  terminal_write_string("Hello, World!");

  // We're done, just hang...
  hcf();
}
