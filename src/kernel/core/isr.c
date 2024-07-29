#include <stdint.h>
#include <stddef.h>

#include <kernel/isr.h>
#include <kernel/panic.h>
#include <kernel/debug_logger.h>

void isr_division_by_zero(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Division by zero error", frame);
}

void isr_debug_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Debug exception", frame);
}

void isr_non_maskable_interrupt_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Non-maskable interrupt exception", frame);
}

void isr_breakpoint_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Breakpoint exception", frame);
}

void isr_into_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("INTO detected overflow exception", frame);
}

void isr_out_of_bounds_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Out of bounds exception", frame);
}

void isr_device_not_available_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Device not available exception", frame);
}

void isr_invalid_opcode_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Invalid opcode exception", frame);
}

void isr_double_fault_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Double fault exception", frame);
}

void isr_coprocessor_segment_overrun_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Coprocessor segment overrun", frame);
}

void isr_invalid_tss_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Bad TSS exception", frame);
}

void isr_segment_not_present_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Segment not present exception", frame);
}

void isr_stack_fault_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Stack fault exception", frame);
}

void isr_general_protection_fault_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("General protection fault exception", frame);
}

void isr_page_fault_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Page fault exception", frame);
}

void isr_floating_point_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Floating point exception", frame);
}

void isr_alignment_check_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Alignment check exception", frame);
}

void isr_machine_check_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Machine check exception", frame);
}

void isr_reserved_exception(InterruptFrame *frame, uint64_t error_code) {
  kernel_panic("Reserved exception", frame);
}

void isr_reserved(InterruptFrame *frame) {
  kernel_panic("Reserved interrupt", frame);
}

// void generic_exception_handler(InterruptFrame* frame, uint64_t vector) {
//   // Log or handle the exception
//   // For example, print to a console or store the state for debugging
//   // printf("Exception occurred! RIP: %p\n", (void*)frame->rip);
//   log_message(
//     &kernel_debug_logger,
//     LOG_ERROR,
//     "Exception triggered. Triggering kernel panic.\n"
//   );

//   kernel_panic(frame);
// }
