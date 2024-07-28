#pragma once

#include <stdint.h>
#include <stddef.h>
#include <ia32.h>

extern void load_gdt(void* gdtr);
extern void reload_segments(void);

typedef struct {
    uint32_t reserved0;
    uint64_t rsp[3];
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed)) TssSegmentDescriptor64;

typedef struct {
  uint16_t limit_low;          // Lower part of the limit
  uint16_t base_low;           // Lower part of the base
  uint8_t base_middle;         // Middle part of the base
  uint8_t access;              // Access flags
  uint8_t limit_high_flags;    // High part of the limit and flags
  uint8_t base_high;           // High part of the base
} __attribute__((packed)) GdtSegmentDescriptor64;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) GDTR;

GdtSegmentDescriptor64 gdt_create_segment_descriptor(
  uint32_t base,
  uint32_t limit,
  uint8_t access,
  uint8_t flags
);

typedef struct {
  uint16_t size;               // Size of the GDT
  uint64_t offset;             // Linear address of the GDT
} __attribute__((packed)) Gdt;

// extern void gdt_reload_segments();
// extern void load_tss(uint16_t tss_selector);
//
// extern void load_gdt(void* gdt_descriptor);
// extern void gdt_reload_segments(void);
// extern void load_tss(uint16_t tss_selector);

void gdt_initialize();

// SEGMENT_DESCRIPTOR_64 gdt_create_segment_descriptor(
//   uint64_t base,
//   uint32_t limit,
//   uint8_t access_byte,
//   uint8_t flags
// );
