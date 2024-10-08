#pragma once

#include <stddef.h>
#include <stdint.h>

#include <hal/idt.h>

/**
 * Load the IDT with interrupt handlers.
 */
void isr_initialize();

/**
 * Common interrupt handler routine (called from assembly)
 */
void isr_handler(InterruptFrame *frame, uint64_t interrupt_number);
