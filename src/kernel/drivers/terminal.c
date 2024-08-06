#include <stdint.h>
#include <stddef.h>

#include <ssfn/ssfn.h>
#include <limine/limine.h>
#include <libk/string.h>

#include <hal/serial.h>
#include <kernel/debug.h>

#include <drivers/terminal.h>

void terminal_clear() {
  for (int i = 0; i < terminal.buffer_size; i++) {
    terminal.buffer[i].character = ' ';
    terminal.buffer[i].fg_color = TerminalColorWhite;
    terminal.buffer[i].bg_color = TerminalColorBlack;
  }

  terminal.cursor_x = 0;
  terminal.cursor_y = 0;
}

void terminal_setcolor(TerminalColor fg) {
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

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "Initializing terminal {width=%d, height=%d, buffer_size=%d}\n",
      terminal.width,
      terminal.height,
      terminal.buffer_size
    );
  }

  // TODO: should actually allocate this dynamically
  #define MAX_TERM_SIZE 28800
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

  terminal.current_fg_color = TerminalColorWhite;
}

void terminal_render() {
  // Clear the entire screen first
  for (int y = 0; y < ssfn_dst.h; y++) {
    for (int x = 0; x < ssfn_dst.w; x++) {
      ((uint32_t*)ssfn_dst.ptr)[y * (ssfn_dst.p / 4) + x] = 0;  // Set to black
    }
  }

  ssfn_dst.x = 0;
  ssfn_dst.y = 0;

  for (int y = 0; y < terminal.height; y++) {
    for (int x = 0; x < terminal.width; x++) {
      CharacterCell cell = terminal.buffer[y * terminal.width + x];
      ssfn_dst.fg = cell.fg_color;
      ssfn_putc(cell.character);
    }

    ssfn_dst.x = 0;
    ssfn_dst.y += 16; // Assuming 16 pixels high characters
  }
}

void terminal_write_char(uint32_t c) {
  if (c == '\n') {
    // Handle newlines
    terminal.cursor_x = 0;
    terminal.cursor_y++;

    if (terminal.cursor_y >= terminal.height) {
      terminal_scroll();
    }

    terminal_render(); // Flush terminal with each newline
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
}

/**
 * This is a wrapper for terminal_write_char that matches the format
 * that stdio.h expects for _putchar (in order for printf to work)
*/
void putchar_(char c) {
  terminal_write_char(c);
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
    terminal.buffer[index].fg_color = TerminalColorWhite;
    terminal.buffer[index].bg_color = TerminalColorBlack;
  }

  // Move cursor up
  terminal.cursor_y--;
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

  // Test case 2: Scrolling
  terminal_write_string("Test Case 2: Scrolling\n");
  for (int i = 0; i < terminal.height - 7; i++) { // 8 doesn't scroll
    terminal_write_string("Line ");
    char num_str[20];
    itoa(i + 1, num_str, 10);
    terminal_write_string(num_str);
    terminal_write_string("\n");
  }
  terminal_write_string("\n");

  // Test case 3: Backspacing
  // terminal_write_string("Test Case 3: Backspacing\n");
  // terminal_write_string("12345");
  // for (int i = 0; i < 3; i++) {
  //   terminal_write_char('\b');
  // }
  // terminal_write_string("abc\n");

  // Edge case: Backspace at beginning of line
  // terminal_write_string("Line 1\n");
  // terminal_write_string("Line 2");
  // for (int i = 0; i < 7; i++) {
  //   terminal_write_char('\b');
  // }
  // terminal_write_string("Replaced\n");

  // Final message
  // terminal_setcolor(TerminalColorGreen);
  // terminal_write_string("\nAll test cases completed.\n");
  // terminal_setcolor(TerminalColorWhite);
}
