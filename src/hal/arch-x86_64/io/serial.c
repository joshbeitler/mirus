#include <hal/serial.h>
#include <libk/string.h>

void serial_initialize() {
  write_byte(COM1 + 1, 0x00);    // Disable all interrupts
  write_byte(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
  write_byte(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
  write_byte(COM1 + 1, 0x00);    //                  (hi byte)
  write_byte(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
  write_byte(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
  write_byte(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received() {
  return read_byte(COM1 + 5) & 1;
}

char serial_read() {
  while (serial_received() == 0);
  return read_byte(COM1);
}

int serial_is_transmit_empty() {
  return read_byte(COM1 + 5) & 0x20;
}

void serial_write_char(char c) {
  while (serial_is_transmit_empty() == 0);
  write_byte(COM1, c);
}

void serial_write_string(const char* str) {
  for (unsigned int i = 0; i < strlen(str); i++) {
    serial_write_char(str[i]);
  }
}

void serial_write_int(int i) {
  char str[32];
  itoa(i, str, 10);
  serial_write_string(str);
}
