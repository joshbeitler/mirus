#pragma once

#include <stdint.h>
#include <stddef.h>

#include <hal/idt.h>

/**
 * Load the IDT with interrupt handlers.
 */
void isr_initialize();

__attribute__((interrupt))
void isr_division_by_zero(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_debug_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_non_maskable_interrupt_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_breakpoint_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_into_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_out_of_bounds_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_device_not_available_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_invalid_opcode_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_double_fault_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_coprocessor_segment_overrun_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_invalid_tss_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_segment_not_present_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_stack_fault_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_general_protection_fault_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_page_fault_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_floating_point_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_alignment_check_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_machine_check_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_reserved_exception(InterruptFrame *frame, uint64_t error_code);

__attribute__((interrupt))
void isr_reserved(InterruptFrame *frame);
