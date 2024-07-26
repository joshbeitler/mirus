#include <stdint.h>
#include <stddef.h>

#include <ssfn.h>
#include <limine.h>

#include <kernel/terminal.h>
#include <kernel/string.h>

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
  if (c == '\n') {
    // Handle newlines
    terminal.cursor_x = 0;
    terminal.cursor_y++;
    // todo: scroll
  } else if (c == '\b') {
    // Handle backspace
    if (terminal.cursor_x > 0) {
      terminal.cursor_x--;
    } else if (terminal.cursor_y > 0) {
      terminal.cursor_y--;
      terminal.cursor_x = terminal.width - 1;
    }

    int index = terminal.cursor_y * terminal.width + terminal.cursor_x;
    terminal.buffer[index].character = ' ';
  } else {
    // Otherwise, write character to buffer normally
    int index = terminal.cursor_y * terminal.width + terminal.cursor_x;
    terminal.buffer[index].character = c;
    terminal.cursor_x++;

    // Wrap if too wide
    if (terminal.cursor_x >= terminal.width) {
      terminal.cursor_x = 0;
      terminal.cursor_y++;
      // TODO: implement scroll here
    }
  }
}

void terminal_write_string(const char* str) {
  for (size_t i = 0; i < strlen(str); i++) {
    terminal_write_char(str[i]);
  }
}

void terminal_setcolor(TerminalColor fg) {
  ssfn_dst.fg = fg;
}

void terminal_scroll() {

}

void terminal_render() {
  // color?
  for (int i = 0; i < terminal.buffer_size; i++) {
    ssfn_putc(terminal.buffer[i].character);
  }

  // store ssfn x and y, then set to cursor pos, draw the cursor, and then restore
  int x = ssfn_dst.x;
  int y = ssfn_dst.y;
  uint32_t fg = ssfn_dst.fg;
  ssfn_dst.x = terminal.cursor_x * 8; // todo make dynamic
  ssfn_dst.y = terminal.cursor_y * 16; // todo make dynamic
  terminal_setcolor(TerminalColorGray);
  ssfn_putc(0x00002588); // unicode32 full block
  terminal_setcolor(fg);
  ssfn_dst.x = x;
  ssfn_dst.y = y;
}
