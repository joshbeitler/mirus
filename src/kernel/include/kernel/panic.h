#pragma once

#include <stdarg.h>

#include <kernel/interrupts.h>

/**
 * Kernel panic, another L for da OG :(
 *
 * @param error_message The error message to display
 * @param frame The interrupt frame at the time of the panic
 */
void kernel_panic(const char* error_message, InterruptFrame* frame);

/**
 * Detailed kernel panic, another L for da OG :(
 *
 * @param error_message The error message to display
 * @param frame The interrupt frame at the time of the panic
 * @param format The format string for the additional information
 * @param ... The additional information
 */
void kernel_panic_detailed(
  const char* error_message,
  InterruptFrame* frame,
  const char *format,
  ...
);

/**
 * Print the interrupt frame
 *
 * @param frame The interrupt frame to print
 */
void print_interrupt_frame(InterruptFrame* frame);

/**
 * Halt and catch fire
 */
void hcf();
