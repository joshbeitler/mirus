#include <stdint.h>
#include <stddef.h>
#include <ssfn.h>

#include <terminal.h>
#include <string.h>

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
