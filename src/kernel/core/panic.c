#include <stdint.h>
#include <stddef.h>

#include <kernel/panic.h>
#include <kernel/isr.h>
#include <kernel/string.h>
#include <kernel/debug_logger.h>
#include <kernel/terminal.h>

void kernel_panic(const char* error_message, InterruptFrame* frame) {
  // log_message(&kernel_debug_logger, LOG_ERROR, "\n\nKernel panic - %s\n", error_message);

  log_message(&kernel_debug_logger, LOG_ERROR, "Kernel panic\n");
  log_message(&kernel_debug_logger, LOG_ERROR, "%s\n\n", error_message);

  log_message(&kernel_debug_logger, LOG_ERROR, "RIP:     0x%lX\n", frame->rip);
  log_message(&kernel_debug_logger, LOG_ERROR, "CS:      0x%lX\n", frame->cs);
  log_message(&kernel_debug_logger, LOG_ERROR, "CFLAGS:  0x%lX\n", frame->rflags);
  log_message(&kernel_debug_logger, LOG_ERROR, "RSP:     0x%lX\n", frame->rsp);
  log_message(&kernel_debug_logger, LOG_ERROR, "SS:      0x%lX\n", frame->ss);

  hcf();
}

void hcf() {
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
