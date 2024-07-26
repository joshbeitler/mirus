#define SSFN_CONSOLEBITMAP_TRUECOLOR

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>
#include <ssfn.h>

#include <string.h>
#include <terminal.h>

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
  .id = LIMINE_FRAMEBUFFER_REQUEST,
  .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.
__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

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

// void ssfn_puts(char* str) {
//   for (size_t i = 0; i < strlen(str); i++) {
//     ssfn_putc(str[i]);
//   }
// }

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
