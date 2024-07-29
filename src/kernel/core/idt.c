#include <stdint.h>
#include <stddef.h>

#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/string.h>
#include <kernel/debug_logger.h>

static IdtSegmentDescriptor idt[IDT_ENTRIES];
static IDTR idtr;

IdtSegmentDescriptor idt_segment_create(
  uint64_t offset,
  uint16_t selector,
  uint8_t type_attr,
  uint8_t ist
) {
  return (IdtSegmentDescriptor) {
    .offset_low = (uint16_t)(offset & 0xFFFF),
    .selector = selector,
    .ist = ist,
    .type_attributes = type_attr,
    .offset_middle = (uint16_t)((offset >> 16) & 0xFFFF),
    .offset_high = (uint32_t)(offset >> 32),
    .must_be_zero = 0
  };
}

void idt_set_entry(int index, void *handler) {
  uint64_t handler_address = (uint64_t)handler;

  idt[index] = idt_segment_create(
    handler_address,
    0x08,
    0x8E,
    0
  );
}

void idt_initialize() {
  log_message(&kernel_debug_logger, LOG_INFO, "  Building IDT entries...");

  memset((unsigned char *)&idt, 0, sizeof(idt)); // Clear whatever was there before
  idt_set_entry(0, isr_division_by_zero);
  idt_set_entry(1, isr_debug_exception);
  idt_set_entry(2, isr_non_maskable_interrupt_exception);
  idt_set_entry(3, isr_breakpoint_exception);
  idt_set_entry(4, isr_into_exception);
  idt_set_entry(5, isr_out_of_bounds_exception);
  idt_set_entry(6, isr_invalid_opcode_exception);
  idt_set_entry(7, isr_device_not_available_exception);
  idt_set_entry(8, isr_double_fault_exception);
  idt_set_entry(9, isr_coprocessor_segment_overrun_exception);
  idt_set_entry(10, isr_invalid_tss_exception);
  idt_set_entry(11, isr_segment_not_present_exception);
  idt_set_entry(12, isr_stack_fault_exception);
  idt_set_entry(13, isr_general_protection_fault_exception);
  idt_set_entry(14, isr_page_fault_exception);
  idt_set_entry(15, isr_reserved_exception);
  idt_set_entry(16, isr_floating_point_exception);
  idt_set_entry(17, isr_alignment_check_exception);
  idt_set_entry(18, isr_machine_check_exception);
  idt_set_entry(19, isr_reserved_exception);
  idt_set_entry(20, isr_reserved_exception);
  idt_set_entry(21, isr_reserved_exception);
  idt_set_entry(22, isr_reserved_exception);
  idt_set_entry(23, isr_reserved_exception);
  idt_set_entry(24, isr_reserved_exception);
  idt_set_entry(25, isr_reserved_exception);
  idt_set_entry(26, isr_reserved_exception);
  idt_set_entry(27, isr_reserved_exception);
  idt_set_entry(28, isr_reserved_exception);
  idt_set_entry(29, isr_reserved_exception);
  idt_set_entry(30, isr_reserved_exception);
  idt_set_entry(31, isr_reserved_exception);
  idt_set_entry(32, isr_reserved_exception);

  log_message(&kernel_debug_logger, LOG_INFO, "done\n");

  log_message(&kernel_debug_logger, LOG_INFO, "  Loading IDT...");
  idtr.limit = sizeof(idt) - 1;
  idtr.base = (uint64_t) &idt;
  idt_load(&idtr);
  log_message(&kernel_debug_logger, LOG_INFO, "done\n");
}
