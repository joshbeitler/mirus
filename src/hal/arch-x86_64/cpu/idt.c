#include <stddef.h>
#include <stdint.h>

#include <hal/hal_logger.h>
#include <hal/idt.h>

#include <libk/string.h>

static IdtSegmentDescriptor idt[IDT_ENTRIES];
static IDTR idtr;

IdtSegmentDescriptor idt_segment_create(
	uint64_t offset, uint16_t selector, uint8_t type_attr, uint8_t ist
) {
	return (IdtSegmentDescriptor
	){.offset_low = (uint16_t)(offset & 0xFFFF),
	  .selector = selector,
	  .ist = ist,
	  .type_attributes = type_attr,
	  .offset_middle = (uint16_t)((offset >> 16) & 0xFFFF),
	  .offset_high = (uint32_t)(offset >> 32),
	  .must_be_zero = 0};
}

void idt_set_entry(int index, void *handler) {
	uint64_t handler_address = (uint64_t)handler;

	idt[index] = idt_segment_create(handler_address, 0x08, 0x8E, 0);
}

void idt_initialize() {
	log_message(&hal_logger, LOG_INFO, "idt", "Building IDT entries\n");

	memset(
		(unsigned char *)&idt, 0, sizeof(idt)
	); // Clear whatever was there before
	for (int i = 0; i < IDT_ENTRIES; i++) {
		idt_set_entry(i, default_interrupt_handler);
	}
	log_message(
		&hal_logger, LOG_INFO, "idt", "Built %d IDT entries\n", IDT_ENTRIES
	);

	log_message(&hal_logger, LOG_INFO, "idt", "Loading IDT\n");
	idtr.limit = sizeof(idt) - 1;
	idtr.base = (uint64_t)&idt;
	idt_load(&idtr);
	log_message(&hal_logger, LOG_INFO, "idt", "IDT loaded\n");
}

void default_interrupt_handler(InterruptFrame *frame) {
	log_message(
		&hal_logger, LOG_INFO, "idt", "Default interrupt handler hit\n"
	);
}
