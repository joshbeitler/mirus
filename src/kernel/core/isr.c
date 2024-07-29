#include <stdint.h>
#include <stddef.h>

#include <kernel/isr.h>
#include <kernel/debug_logger.h>

void generic_exception_handler(InterruptFrame* frame) {
  // Log or handle the exception
  // For example, print to a console or store the state for debugging
  // printf("Exception occurred! RIP: %p\n", (void*)frame->rip);
  log_message(&kernel_debug_logger, LOG_ERROR, "\n\nanother L for da og\n\n");

  // Halt and catch fire.
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
