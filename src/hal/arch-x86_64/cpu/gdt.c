#include <stdint.h>
#include <stddef.h>

#include <hal/gdt.h>
#include <hal/hal_logger.h>

static GdtSegmentDescriptor gdt[GDT_ENTRIES];
static GDTR gdtr;

static TssSegmentDescriptor tss;

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

void gdt_initialize(uintptr_t kernel_stack_ptr) {
  log_message(&hal_logger, LOG_INFO, "  Building GDT entries\n");

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

  // Set up the TSS
  log_message(&hal_logger, LOG_INFO, "  Building TSS entries\n");
  tss.iomap_base = sizeof(tss);
  tss.rsp[0] = kernel_stack_ptr;

  // Create the TSS descriptor
  uint64_t tss_base = (uint64_t)&tss;
  uint32_t tss_limit = sizeof(tss) - 1;

  gdt[5] = (GdtSegmentDescriptor) {
    .limit_low = tss_limit & 0xFFFF,
    .base_low = tss_base & 0xFFFF,
    .base_middle = (tss_base >> 16) & 0xFF,
    .access = ACCESS_TSS,
    .limit_high_flags = ((tss_limit >> 16) & 0x0F) | 0x00,
    .base_high = (tss_base >> 24) & 0xFF
  };

  gdt[6] = (GdtSegmentDescriptor) {
    .limit_low = (tss_base >> 32) & 0xFFFF,
    .base_low = (tss_base >> 48) & 0xFFFF,
    .base_middle = 0,
    .access = 0,
    .limit_high_flags = 0,
    .base_high = 0
  };
  log_message(&hal_logger, LOG_INFO, "  TSS entries built\n");

  log_message(&hal_logger, LOG_INFO, "  GDT entries built\n");

  log_message(&hal_logger, LOG_INFO, "  Loading GDT\n");
  gdtr.limit = sizeof(gdt) - 1;
  gdtr.base = (uint64_t) &gdt;
  gdt_load(&gdtr);
  log_message(&hal_logger, LOG_INFO, "  GDT loaded\n");

  log_message(&hal_logger, LOG_INFO, "  Reloading segments\n");
  gdt_reload_segments();
  log_message(&hal_logger, LOG_INFO, "  Segments reloaded\n");

  log_message(&hal_logger, LOG_INFO, "  Loading TSS\n");
  asm volatile("ltr %%ax" : : "a" (0x28)); // 0x28 is the offset of TSS in GDT (5 * 8)
  log_message(&hal_logger, LOG_INFO, "  TSS loaded\n");
}
