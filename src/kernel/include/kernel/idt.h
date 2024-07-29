#pragma once

#include <stdint.h>
#include <stddef.h>

#define IDT_ENTRIES 256

/**
 * Represents a IDT segment descriptor in long mode
 */
typedef struct {
  uint16_t offset_low;      // Lower 16 bits of offset to procedure entry point
  uint16_t code_segment;    // Segment selector for destination code segment
  uint8_t ist;              // Index into the TSS Interrupt Stack Table
  uint8_t type_attributes;  // Type and attributes
  uint16_t offset_middle;   // Middle 16 bits of offset
  uint32_t offset_high;     // Higher 32 bits of offset
  uint32_t must_be_zero;    // Reserved, must be zero
} __attribute__((packed)) IdtSegmentDescriptor;

/**
 * Represents the IDTR register
 */
typedef struct {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed)) IDTR;

/**
 * Creates a IDT segment descriptor
 *
*/
/**
 * Creates and returns an IDT segment descriptor.
 *
 * @param offset The 64-bit offset to the procedure entry point
 * @param selector Segment selector for the code segment
 * @param type_attr Type and attributes for the descriptor
 * @param ist Index to Interrupt Stack Table
 * @return An initialized IDT segment descriptor.
 */
IdtSegmentDescriptor idt_segment_create(
  uint64_t offset,
  uint16_t selector,
  uint8_t type_attr,
  uint8_t ist
);

/**
 * Initializes the IDT for 64-bit long mode
*/
void idt_initialize();

/**
 * Calls the LIDT instruction with the given IDTR
 *
 * @param gdtr The IDTR to load
 */
extern void idt_load(void* idtr);
