#include <stdint.h>
#include <stddef.h>

#include <ia32.h>
#include <kernel/gdt.h>
#include <kernel/serial.h>

#define GDT_ENTRIES 6

// TASK_STATE_SEGMENT_64 task_state_segment = {0};
// SEGMENT_DESCRIPTOR_64 gdt[GDT_ENTRIES];
// SEGMENT_DESCRIPTOR_REGISTER_64 gdt_descriptor;
//
// typedef struct {
//   uint16_t limit_low;
//   uint16_t base_low;
//   uint8_t base_middle;
//   uint8_t flags;
//   uint8_t limit_high;
//   uint8_t granularity;
//   uint8_t base_high;
// } __attribute__((packed)) GdtSegmentDescriptor64;
//
#define SEGMENT_BASE  0x0
#define SEGMENT_LIMIT 0xFFFFF
#define CODE_ACCESS   0x9A   // Code segment: Execute/Read, accessed
#define DATA_ACCESS   0x92   // Data segment: Read/Write, accessed
#define FLAGS         0xC    // Page granularity, 32-bit protected mode

#define TSS_ADDRESS 0x00   // Replace 0x00 with the actual memory address of the TSS

static GdtSegmentDescriptor64 gdt[GDT_ENTRIES];
static GDTR gdtr;

// struct {
//     uint16_t limit;
//     uint64_t base;
// } __attribute__((packed)) gdtr = {
//     .limit = sizeof(gdt) - 1,
//     .base = (uint64_t)&gdt
// };

GdtSegmentDescriptor64 gdt_create_segment_descriptor(
  uint32_t base,
  uint32_t limit,
  uint8_t access,
  uint8_t flags
) {
  return (GdtSegmentDescriptor64){
      .limit_low = limit & 0xFFFF,
      .base_low = base & 0xFFFF,
      .base_middle = (base >> 16) & 0xFF,
      .access = access,
      .limit_high_flags = ((limit >> 16) & 0x0F) | (flags << 4),
      .base_high = (base >> 24) & 0xFF
  };
}

void gdt_initialize() {
  // serial_write_string("Building GDT entries...");
  gdt[0] = gdt_create_segment_descriptor(0, 0x00000, 0x00, 0x0);  // Null descriptor
  gdt[1] = gdt_create_segment_descriptor(0, 0xFFFFF, 0x9A, 0xA);  // Kernel Mode Code Segment
  gdt[2] = gdt_create_segment_descriptor(0, 0xFFFFF, 0x92, 0xC);  // Kernel Mode Data Segment
  gdt[3] = gdt_create_segment_descriptor(0, 0xFFFFF, 0xFA, 0xA);  // User Mode Code Segment
  gdt[4]= gdt_create_segment_descriptor(0, 0xFFFFF, 0xF2, 0xC);  // User Mode Data Segment
  gdt[5] = gdt_create_segment_descriptor(TSS_ADDRESS, sizeof(TssSegmentDescriptor64) - 1, 0x89, 0x0); // TSS

  gdtr.limit = sizeof(gdt) - 1;
  gdtr.base = (uint64_t) &gdt;

  load_gdt(&gdtr);
  reload_segments();
  // gdt[0] = gdt_create_segment_descriptor(0, 0x00000000, 0x00, 0x0);  // Null Descriptor
  // gdt[1] = gdt_create_segment_descriptor(0, 0xFFFFF, 0x9A, 0xA);     // Kernel Mode Code Segment
  // gdt[2] = gdt_create_segment_descriptor(0, 0xFFFFF, 0x92, 0xC);     // Kernel Mode Data Segment
  // gdt[3] = gdt_create_segment_descriptor(0, 0xFFFFF, 0xFA, 0xA);     // User Mode Code Segment
  // gdt[4] = gdt_create_segment_descriptor(0, 0xFFFFF, 0xF2, 0xC);     // User Mode Data Segment
  // gdt[5] = gdt_create_segment_descriptor((uint64_t)&task_state_segment, sizeof(task_state_segment) - 1, 0x89, 0x0);  // TSS

  // serial_write_string("done.\n");
  // serial_write_string("Building GDT...");
  // // gdt_descriptor.Limit = sizeof(gdt) - 1;
  // // gdt_descriptor.BaseAddress = (uint64_t)&gdt;
  // serial_write_string("done.\n");

  // serial_write_string("Loading GDT...");
  // // TODO: implement in asm
  // // load_gdt(&gdt_descriptor);
  // serial_write_string("done.\n");

  // serial_write_string("Reloading GDT segments...");
  // // gdt_reload_segments();
  // serial_write_string("done.\n");

  // serial_write_string("Loading TSS...");
  // // load_tss(0x28);
  // serial_write_string("done.\n");
}
