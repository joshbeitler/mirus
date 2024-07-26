#include <stdint.h>
#include <stddef.h>

struct CursorPosition {
  int x;
  int y;
};

void terminal_write_char(uint32_t c);
void terminal_write_string(char* str);
void clear();
