#include <stdint.h>
#include <stddef.h>
#include <ssfn.h>
#include <limine.h>

#include <terminal.h>
#include <string.h>

void terminal_clear() {
  for (int i = 0; i < terminal.buffer_size; i++) {
    terminal.buffer[i].character = ' ';
    terminal.buffer[i].fg_color = TerminalColorWhite;
    terminal.buffer[i].bg_color = TerminalColorBlack;
  }

  terminal.cursor_x = 0;
  terminal.cursor_y = 0;
}

void terminal_initialize(
  struct limine_file *font_file,
  struct limine_framebuffer *framebuffer
) {
  // Set up terminal state
  terminal.width = framebuffer->width / 8;
  terminal.height = framebuffer->height / 16;
  terminal.cursor_x = 0;
  terminal.cursor_y = 0;
  terminal.buffer_size = terminal.width * terminal.height;

  // TODO: should actually allocate this dynamically
  #define MAX_TERM_SIZE 10000
  static CharacterCell static_buffer[MAX_TERM_SIZE];
  terminal.buffer = static_buffer;
  terminal_clear();

  // Set up font renderer
  ssfn_src = font_file->address;

  ssfn_dst.ptr = framebuffer->address;
  ssfn_dst.w = framebuffer->width;
  ssfn_dst.h = framebuffer->height;
  ssfn_dst.p = framebuffer->pitch;
  ssfn_dst.x = terminal.cursor_x; // When we move the cursor later, this multiplies by character width
  ssfn_dst.y = terminal.cursor_y; // When we move the cursor later, this multiplies by character width
  ssfn_dst.fg = TerminalColorWhite;
}

void terminal_write_char(uint32_t c) {
  int index = terminal.cursor_y * terminal.width + terminal.cursor_x;
  terminal.buffer[index].character = c;
  terminal.cursor_x++;

  // Wrap if too wide
  if (terminal.cursor_x >= terminal.width) {
    terminal.cursor_x = 0;
    terminal.cursor_y++;
    // TODO: implement scroll here
  }

  // TODO: this goes in the render function
  // ssfn_putc(c);
}

void terminal_write_string(const char* str) {
  for (size_t i = 0; i < strlen(str); i++) {
    terminal_write_char(str[i]);
  }
}

void terminal_setcolor(TerminalColor fg) {
  ssfn_dst.fg = fg;
}
