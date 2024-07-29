#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} InterruptFrame;

__attribute__((interrupt))
void generic_exception_handler(InterruptFrame* frame);

// __attribute__((interrupt))
// void isr_division_by_zero(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_debug_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_non_maskable_interrupt_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_int3_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_into_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_out_of_bounds_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_invalid_opcode_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_coprocessor_na_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_double_fault_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_coprocessor_segment_overrun_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_bad_tss_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_segment_not_present_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_stack_fault_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_general_protection_fault_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_page_fault_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_reserved_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_floating_point_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_alignment_check_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_machine_check_exception(InterruptFrame *frame, uint64_t error_code);

// __attribute__((interrupt))
// void isr_reserved(InterruptFrame *frame);
