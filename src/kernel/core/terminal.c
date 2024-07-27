#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <ssfn.h>
#include <limine.h>

#include <kernel/terminal.h>
#include <kernel/string.h>
#include <kernel/serial.h>

void terminal_clear() {
  for (int i = 0; i < terminal.buffer_size; i++) {
    terminal.buffer[i].character = ' ';
    terminal.buffer[i].fg_color = TerminalColorWhite;
    terminal.buffer[i].bg_color = TerminalColorBlack;
  }

  terminal.cursor_x = 0;
  terminal.cursor_y = 0;
  terminal.needs_full_redraw = true;
  terminal.last_rendered_line = 0;
}

void terminal_setcolor(uint32_t fg) {
  terminal.current_fg_color = fg;
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

  // Set up font renderer
  ssfn_src = font_file->address;
  ssfn_dst.ptr = framebuffer->address;
  ssfn_dst.w = framebuffer->width;
  ssfn_dst.h = framebuffer->height;
  ssfn_dst.p = framebuffer->pitch;
  ssfn_dst.fg = TerminalColorWhite;

  terminal.current_fg_color = TerminalColorWhite;
  terminal.needs_full_redraw = true;
  terminal.last_rendered_line = 0;

  terminal_clear();
}

void terminal_render() {
  if (terminal.needs_full_redraw) {
    // Clear the entire screen
    for (int y = 0; y < ssfn_dst.h; y++) {
      for (int x = 0; x < ssfn_dst.w; x++) {
        ((uint32_t*)ssfn_dst.ptr)[y * (ssfn_dst.p / 4) + x] = 0;  // Set to black
      }
    }

    terminal.needs_full_redraw = false;
    terminal.last_rendered_line = 0;
  }

  // Render only from the last rendered line to the current cursor position
  int start_line = terminal.last_rendered_line;
  int end_line = (terminal.cursor_y > start_line) ? terminal.cursor_y : start_line;

  ssfn_dst.x = 0;
  ssfn_dst.y = start_line * 16;  // Assuming 16 pixels high characters

  for (int y = start_line; y <= end_line; y++) {
    for (int x = 0; x < terminal.width; x++) {
      CharacterCell cell = terminal.buffer[y * terminal.width + x];
      ssfn_dst.fg = cell.fg_color;
      ssfn_putc(cell.character);
    }

    ssfn_dst.x = 0;
    ssfn_dst.y += 16;
  }

  terminal.last_rendered_line = end_line;
}

void terminal_write_char(uint32_t c) {
  if (c == '\n') {
    // Handle newlines
    terminal.cursor_x = 0;
    terminal.cursor_y++;
    if (terminal.cursor_y >= terminal.height) {
      terminal_scroll();
    }
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
    terminal.buffer[index].fg_color = terminal.current_fg_color;
    terminal.cursor_x++;

    // Wrap if too wide
    if (terminal.cursor_x >= terminal.width) {
      terminal.cursor_x = 0;
      terminal.cursor_y++;
      if (terminal.cursor_y >= terminal.height) {
        terminal_scroll();
      }
    }
  }

  if (terminal.cursor_y > terminal.last_rendered_line) {
    terminal.last_rendered_line = terminal.cursor_y;
  }
}

void terminal_write_string(const char* str) {
  for (size_t i = 0; i < strlen(str); i++) {
    terminal_write_char(str[i]);
  }
}

void terminal_scroll() {
  // Move everything up one line
  for (int y = 0; y < terminal.height - 1; y++) {
    for (int x = 0; x < terminal.width; x++) {
      terminal.buffer[y * terminal.width + x] = terminal.buffer[(y + 1) * terminal.width + x];
    }
  }

  // Clear the last line
  for (int x = 0; x < terminal.width; x++) {
    int index = (terminal.height - 1) * terminal.width + x;
    terminal.buffer[index].character = ' ';
    terminal.buffer[index].fg_color = terminal.current_fg_color;
    terminal.buffer[index].bg_color = TerminalColorBlack;
  }

  terminal.cursor_y--;
  terminal.needs_full_redraw = true;
  terminal.last_rendered_line = terminal.height - 1;
}

void terminal_flush() {
  terminal_render();
}

void test_terminal() {
  // Test case 1: Multi-line string with different colors
  terminal_write_string("Test Case 1: Multi-line string with different colors\n");
  terminal_setcolor(TerminalColorRed);
  terminal_write_string("This line is red\n");
  terminal_setcolor(TerminalColorGreen);
  terminal_write_string("This line is green\n");
  terminal_setcolor(TerminalColorBlue);
  terminal_write_string("This line is blue\n");
  terminal_setcolor(TerminalColorWhite);
  terminal_write_string("\n");
  terminal_flush();

  // Test case 2: Scrolling
  terminal_write_string("Test Case 2: Scrolling\n");
  for (int i = 0; i < terminal.height + 5; i++) {
    terminal_write_string("Line ");
    char num_str[20];
    itoa(i + 1, num_str, 10);
    terminal_write_string(num_str);
    terminal_write_string("\n");
    if (i % 5 == 0) terminal_flush();  // Render every 5 lines
  }
  terminal_flush();

  // Test case 3: Backspacing
  terminal_write_string("Test Case 3: Backspacing\n");
  terminal_write_string("12345");
  for (int i = 0; i < 3; i++) {
    terminal_write_char('\b');
  }
  terminal_write_string("abc\n");
  terminal_flush();

  // Edge case: Backspace at beginning of line
  terminal_write_string("Line 1\n");
  terminal_write_string("Line 2");
  for (int i = 0; i < 7; i++) {
    terminal_write_char('\b');
  }
  terminal_write_string("Replaced\n");
  terminal_flush();

  // Final message
  terminal_setcolor(TerminalColorGreen);
  terminal_write_string("\nAll test cases completed.\n");
  terminal_setcolor(TerminalColorWhite);
  terminal_flush();
}
