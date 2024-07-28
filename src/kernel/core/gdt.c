#include <stdint.h>
#include <stddef.h>

#include <kernel/gdt.h>
#include <kernel/serial.h>

#define GDT_ENTRIES 6

#define SEGMENT_BASE  0x0
#define SEGMENT_LIMIT 0xFFFFF
#define CODE_ACCESS   0x9A   // Code segment: Execute/Read, accessed
#define DATA_ACCESS   0x92   // Data segment: Read/Write, accessed
#define FLAGS         0xC    // Page granularity, 32-bit protected mode

#define TSS_ADDRESS 0x00   // Replace 0x00 with the actual memory address of the TSS

static GdtSegmentDescriptor gdt[GDT_ENTRIES];
static GDTR gdtr;

GdtSegmentDescriptor gdt_create_segment_descriptor(
  uint32_t base,
  uint32_t limit,
  uint8_t access,
  uint8_t flags
) {
  return (GdtSegmentDescriptor){
    .limit_low = limit & 0xFFFF,
    .base_low = base & 0xFFFF,
    .base_middle = (base >> 16) & 0xFF,
    .access = access,
    .limit_high_flags = ((limit >> 16) & 0x0F) | (flags << 4),
    .base_high = (base >> 24) & 0xFF
  };
}

void gdt_initialize() {
  serial_write_string("  Building GDT entries...");
  gdt[0] = gdt_create_segment_descriptor(0, 0x00000, 0x00, 0x0);  // Null descriptor
  gdt[1] = gdt_create_segment_descriptor(0, 0xFFFFF, 0x9A, 0xA);  // Kernel Mode Code Segment
  gdt[2] = gdt_create_segment_descriptor(0, 0xFFFFF, 0x92, 0xC);  // Kernel Mode Data Segment
  gdt[3] = gdt_create_segment_descriptor(0, 0xFFFFF, 0xFA, 0xA);  // User Mode Code Segment
  gdt[4]= gdt_create_segment_descriptor(0, 0xFFFFF, 0xF2, 0xC);  // User Mode Data Segment
  gdt[5] = gdt_create_segment_descriptor(TSS_ADDRESS, sizeof(TssSegmentDescriptor) - 1, 0x89, 0x0); // TSS
  serial_write_string("done\n");

  serial_write_string("  Building GDT...");
  gdtr.limit = sizeof(gdt) - 1;
  gdtr.base = (uint64_t) &gdt;
  serial_write_string("done\n");

  serial_write_string("  Loading GDT...");
  load_gdt(&gdtr);
  serial_write_string("done\n");

  serial_write_string("  Reloading segments...");
  reload_segments();
  serial_write_string("done\n");

  // serial_write_string("Loading TSS...");
  // // load_tss(0x28);
  // serial_write_string("done.\n");
}
