#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Defines a single character on the terminal, with associated color data.
 * Character is a Unicode codepoint, and colors are 32-bit ARGB values.
 */
typedef struct {
	uint32_t character;
	uint32_t fg_color;
	uint32_t bg_color;
} CharacterCell;

/**
 * Defines a terminal with a buffer of character cells, as well as the current
 * cursor position. Used for writing to the framebuffer.
 */
typedef struct {
	CharacterCell *buffer;
	int buffer_size;
	int height;
	int width;
	int cursor_x;
	int cursor_y;
	uint32_t current_fg_color;
} Terminal;

static Terminal terminal = {0};

/**
 * Pre-defined hex colors for use in the terminal.
 */
typedef enum {
	TerminalColorWhite = 0xFFFFFF,
	TerminalColorBlack = 0x000000,
	TerminalColorGray = 0x808080,
	TerminalColorRed = 0xFF0000,
	TerminalColorOrange = 0xFFA500,
	TerminalColorYellow = 0xFFFF00,
	TerminalColorGreen = 0x00FF00,
	TerminalColorBlue = 0x0000FF,
	TerminalColorPurple = 0x800080,
	TerminalColorPink = 0xFFC0CB,
	TerminalColorCyan = 0x00FFFF,
} TerminalColor;

/**
 * Initializes the terminal. Will set up the font as well as terminal state
 * variables like the cursor position and terminal size.
 *
 * @param font_file The SSFN font file to use.
 * @param framebuffer The GOP framebuffer to write to.
 */
void terminal_initialize(
	struct limine_file *font_file, struct limine_framebuffer *framebuffer
);

/**
 * Writes a character to the terminal.
 *
 * @param c The character to write.
 */
void terminal_write_char(uint32_t c);

/**
 * Writes a string to the terminal.
 *
 * @param str The string to write.
 */
void terminal_write_string(const char *str);

/**
 * Sets the terminal foreground color.
 *
 * @param fg The new foreground color.
 */
void terminal_set_color(TerminalColor fg);

/**
 * Clears the terminal by directly clearing the framebuffer.
 */
void terminal_clear();

/**
 * Renders the virtual terminal buffer to the framebuffer.
 */
void terminal_render();

void terminal_scroll();
void test_terminal();
