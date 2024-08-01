#pragma once

#include <stdint.h>
#include <stddef.h>

#define GDT_ENTRIES 7
#define SEGMENT_BASE 0x0
#define SEGMENT_LIMIT 0xFFFFF

// Access rights
#define ACCESS_KERNEL_CODE 0x9A  // Execute/Read, accessed, ring 0
#define ACCESS_KERNEL_DATA 0x92  // Read/Write, accessed, ring 0
#define ACCESS_USER_CODE 0xFA    // Execute/Read, accessed, ring 3
#define ACCESS_USER_DATA 0xF2    // Read/Write, accessed, ring 3
#define ACCESS_TSS 0x89           // TSS, ring 0

// Flags
#define FLAG_GRANULARITY_4KB 0xC // Granularity 4KB, 32-bit opcode
#define FLAG_GRANULARITY_BYTE 0xA // Granularity byte, 32-bit opcode

/**
  * Represents a TSS segment descriptor
 */
typedef struct {
  uint32_t reserved1;
  uint64_t rsp[3];
  uint64_t reserved2;
  uint64_t ist[7];
  uint64_t reserved3;
  uint16_t reserved4;
  uint16_t iomap_base;
} __attribute__((packed)) TssSegmentDescriptor;

/**
 * Represents a GDT segment descriptor in long mode
 */
typedef struct {
  uint16_t limit_low;          // Lower part of the limit
  uint16_t base_low;           // Lower part of the base
  uint8_t base_middle;         // Middle part of the base
  uint8_t access;              // Access flags
  uint8_t limit_high_flags;    // High part of the limit and flags
  uint8_t base_high;           // High part of the base
} __attribute__((packed)) GdtSegmentDescriptor;

/**
 * Represents the GDTR register
 */
typedef struct {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed)) GDTR;

/**
 * Creates a GDT segment descriptor
 *
 * @param base The base address of the segment
 * @param limit The limit of the segment
 * @param access The access flags of the segment
 * @param flags The flags of the segment
*/
GdtSegmentDescriptor gdt_create_segment_descriptor(
  uint32_t base,
  uint32_t limit,
  uint8_t access,
  uint8_t flags
);

/**
 * Initializes the GDT for 64-bit long mode
*/
void gdt_initialize(uintptr_t kernel_stack_ptr);

/**
 * Calls the LGDT instruction with the given GDTR
 *
 * @param gdtr The GDTR to load
 */
extern void gdt_load(void* gdtr);

/**
 * Reloads the segment registers with the current values in the GDT
 */
extern void gdt_reload_segments();
