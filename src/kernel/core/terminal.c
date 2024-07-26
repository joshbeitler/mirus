#include <stdint.h>
#include <stddef.h>
#include <ssfn.h>
#include <limine.h>

#include <terminal.h>
#include <string.h>

/**
 * Pre-defined hex colors for use in the terminal.
 */
enum TerminalColor {
  TerminalColorWhite = 0xFFFFFF,
  TerminalColorRed = 0xFF0000,
  TerminalColorGreen = 0x00FF00,
  TerminalColorBlue = 0x0000FF,
  TerminalColorOrange = 0xFFA500,
  TerminalColorYellow = 0xFFFF00,
  TerminalColorPink = 0xFFC0CB,
  TerminalColorGray = 0x808080
};

/**
 * Initializes the terminal.
 *
 * @param font_file The SSFN font file to use.
 * @param framebuffer The GOP framebuffer to write to.
 */
void terminal_initialize(struct limine_file *font_file, struct limine_framebuffer *framebuffer) {
  ssfn_src = font_file->address;

  ssfn_dst.ptr = framebuffer->address;
  ssfn_dst.w = framebuffer->width;
  ssfn_dst.h = framebuffer->height;
  ssfn_dst.p = framebuffer->pitch;
  ssfn_dst.x = ssfn_dst.y = 0;
  ssfn_dst.fg = TerminalColorGray;
}

/**
 * Writes a character to the terminal.
 *
 * @param c The character to write.
 */
 void terminal_write_char(uint32_t c) {
  ssfn_putc(c);
}

/**
 * Writes a string to the terminal.
 *
 * @param str The string to write.
 */
void terminal_write_string(char* str) {
  for (size_t i = 0; i < strlen(str); i++) {
    terminal_write_char(str[i]);
  }
}

/**
 * Clears the terminal by directly clearing the framebuffer.
 */
void clear() {
}
