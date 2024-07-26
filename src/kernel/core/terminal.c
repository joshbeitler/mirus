#include <stdint.h>
#include <stddef.h>
#include <ssfn.h>
#include <limine.h>

#include <terminal.h>
#include <string.h>

enum TerminalColor {
  TerminalColorWhite = 0xFFFFFF
};

void terminal_initialize(struct limine_file *font_file, struct limine_framebuffer *framebuffer) {
  ssfn_src = font_file->address;

  ssfn_dst.ptr = framebuffer->address;
  ssfn_dst.w = framebuffer->width;
  ssfn_dst.h = framebuffer->height;
  ssfn_dst.p = framebuffer->pitch;
  ssfn_dst.x = ssfn_dst.y = 0;
  ssfn_dst.fg = TerminalColorWhite;
}

void terminal_write_char(uint32_t c) {
  ssfn_putc(c);
}

void terminal_write_string(char* str) {
  for (size_t i = 0; i < strlen(str); i++) {
    terminal_write_char(str[i]);
  }
}

void clear() {

}
