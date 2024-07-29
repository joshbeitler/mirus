#include <stdint.h>
#include <stddef.h>

#include <hal/serial.h>
#include <libk/string.h>
#include <printf/printf.h>

#include <kernel/debug_logger.h>
#include <kernel/panic.h>
#include <kernel/isr.h>
#include <kernel/terminal.h>

// TODO: I need to double check with the debugger that the display of the
// registers is correct.

/**
 * For some reason, the kernel debug logger doesn't like to work in here. This
 * is despite the fact that the serial logger works just fine, and so does
 * printf, sprintf etc. So we're just doing that manually for now until I can
 * figure out why.
 */
void kernel_panic(const char* error_message, InterruptFrame* frame) {
  terminal_clear();
  serial_write_string("[FATAL] Kernel panic encountered\n");

  // use sprintf, it doesn't matter if its unsafe, the system is going down
  char buffer[1024];
  sprintf_(buffer, "[FATAL] %s\n", error_message);
  serial_write_string(buffer);

  // use sprintf to format registers and output to serial
  sprintf_(buffer, "[FATAL] RIP:     0x%lX\n", frame->rip);
  serial_write_string(buffer);
  sprintf_(buffer, "[FATAL] CS:      0x%lX\n", frame->cs);
  serial_write_string(buffer);
  sprintf_(buffer, "[FATAL] CFLAGS:  0x%lX\n", frame->rflags);
  serial_write_string(buffer);
  sprintf_(buffer, "[FATAL] RSP:     0x%lX\n", frame->rsp);
  serial_write_string(buffer);
  sprintf_(buffer, "[FATAL] SS:      0x%lX\n", frame->ss);
  serial_write_string(buffer);

  printf_("Kernel panic!\n");
  printf_("%s\n\n", error_message);

  printf_("RIP:     0x%lX\n", frame->rip);
  printf_("CS:      0x%lX\n", frame->cs);
  printf_("CFLAGS:  0x%lX\n", frame->rflags);
  printf_("RSP:     0x%lX\n", frame->rsp);
  printf_("SS:      0x%lX\n", frame->ss);

  printf_("\nHalting and catching fire\n");
  serial_write_string("[FATAL] Halting and catching fire\n");

  hcf();
}

void hcf() {
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
