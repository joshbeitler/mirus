#include <stdint.h>
#include <stddef.h>

#include <kernel/gdt.h>
#include <kernel/debug_logger.h>

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
  log_message(&kernel_debug_logger, LOG_INFO, "  Building GDT entries...");

  // Null descriptor
  gdt[0] = gdt_create_segment_descriptor(0, 0, 0x00, 0x0);

  // Kernel Mode Code Segment
  gdt[1] = gdt_create_segment_descriptor(
    SEGMENT_BASE,
    SEGMENT_LIMIT,
    ACCESS_KERNEL_CODE,
    FLAG_GRANULARITY_BYTE
  );

  // Kernel Mode Data Segment
  gdt[2] = gdt_create_segment_descriptor(
    SEGMENT_BASE,
    SEGMENT_LIMIT,
    ACCESS_KERNEL_DATA,
    FLAG_GRANULARITY_4KB
  );

  // User Mode Code Segment
  gdt[3] = gdt_create_segment_descriptor(
    SEGMENT_BASE,
    SEGMENT_LIMIT,
    ACCESS_USER_CODE,
    FLAG_GRANULARITY_BYTE
  );

  // User Mode Data Segment
  gdt[4] = gdt_create_segment_descriptor(
    SEGMENT_BASE,
    SEGMENT_LIMIT,
    ACCESS_USER_DATA,
    FLAG_GRANULARITY_4KB
  );

  // TSS
  gdt[5] = gdt_create_segment_descriptor(
    TSS_ADDRESS,
    sizeof(TssSegmentDescriptor) - 1,
    ACCESS_TSS,
    0x0
  );

  log_message(&kernel_debug_logger, LOG_INFO, "done\n");

  log_message(&kernel_debug_logger, LOG_INFO, "  Loading GDT...");
  gdtr.limit = sizeof(gdt) - 1;
  gdtr.base = (uint64_t) &gdt;
  gdt_load(&gdtr);
  log_message(&kernel_debug_logger, LOG_INFO, "done\n");

  log_message(&kernel_debug_logger, LOG_INFO, "  Reloading segments...");
  gdt_reload_segments();
  log_message(&kernel_debug_logger, LOG_INFO, "done\n");

  // serial_write_string("Loading TSS...");
  // // load_tss(0x28);
  // serial_write_string("done.\n");
}
