#pragma once

#include <stdint.h>
#include <stddef.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8
#define COM5 0x5F8
#define COM6 0x4F8
#define COM7 0x5E8
#define COM8 0x4E8

/**
 * Read from an IO port
 *
 * @param port Port to read
 * @return Value of port
 */
static inline unsigned char read_byte(unsigned short port) {
  unsigned char rv;
  asm volatile("inb %1, %0" : "=a" (rv) : "dN" (port));

  return rv;
}

/**
 * Write to an IO port
 *
 * @param port Port to write
 * @param data Data to write
 */
static inline void write_byte(unsigned short port, unsigned char data) {
  asm volatile("outb %1, %0" : : "dN" (port), "a" (data));
}

void serial_initialize();
char serial_read();
int serial_recieved();
int serial_is_transmit_empty();
void serial_write_char(char c);
void serial_write_string(const char* str);
