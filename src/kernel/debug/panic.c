#include <stdint.h>
#include <stddef.h>

#include <hal/serial.h>
#include <libk/string.h>
#include <printf/printf.h>

#include <kernel/interrupts.h>
#include <kernel/debug.h>
#include <kernel/panic.h>

#include <drivers/terminal.h>

// TODO: emit a detailed log with log_message and jems
// TODO: support extra details via a va_list or similar

void kernel_panic(const char* error_message, InterruptFrame* frame) {
  log_message(&kernel_debug_logger, LOG_FATAL, "kernel", "Kernel panic encountered\n");

  terminal_clear();

  printf_("Kernel panic!\n");
  printf_("%s\n\n", error_message);

  printf_("RIP:     0x%lX\n", frame->rip);
  printf_("CS:      0x%lX\n", frame->cs);
  printf_("CFLAGS:  0x%lX\n", frame->rflags);
  printf_("RSP:     0x%lX\n", frame->rsp);
  printf_("SS:      0x%lX\n", frame->ss);

  printf_("\nHalting and catching fire\n");

  hcf();
}

void hcf() {
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
