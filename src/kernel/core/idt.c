#include <stdint.h>
#include <stddef.h>

#include <kernel/idt.h>
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
    .code_segment = selector,
    .ist = ist,
    .type_attributes = type_attr,
    .offset_middle = (uint16_t)((offset >> 16) & 0xFFFF),
    .offset_high = (uint32_t)(offset >> 32),
    .must_be_zero = 0
  };
}

void idt_initialize() {
  log_message(&kernel_debug_logger, LOG_INFO, "  Building IDT entries...");
  //
  log_message(&kernel_debug_logger, LOG_INFO, "done\n");

  log_message(&kernel_debug_logger, LOG_INFO, "  Loading IDT...");
  idtr.limit = sizeof(idt) - 1;
  idtr.base = (uint64_t) &idt;
  memset((unsigned char *)&idt, 0, sizeof(idt));
  idt_load(&idtr);
  log_message(&kernel_debug_logger, LOG_INFO, "done\n");
}
