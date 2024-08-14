#include <stdint.h>
#include <stddef.h>

#include <hal/idt.h>

#include <kernel/interrupts.h>
#include <kernel/panic.h>
#include <kernel/debug.h>

// TODO: put somewhere else
static uint64_t read_cr2() {
  uint64_t value;
  __asm__ volatile ("mov %%cr2, %0" : "=r" (value));
  return value;
}

// Declare ISR handlers
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

void isr_initialize() {
  log_message(&kernel_debug_logger, LOG_INFO, "interrupts", "Registering ISRs\n");

  idt_set_entry(0, isr0);
  idt_set_entry(1, isr1);
  idt_set_entry(2, isr2);
  idt_set_entry(3, isr3);
  idt_set_entry(4, isr4);
  idt_set_entry(5, isr5);
  idt_set_entry(6, isr6);
  idt_set_entry(7, isr7);
  idt_set_entry(8, isr8);
  idt_set_entry(9, isr9);
  idt_set_entry(10, isr10);
  idt_set_entry(11, isr11);
  idt_set_entry(12, isr12);
  idt_set_entry(13, isr13);
  idt_set_entry(14, isr14);
  idt_set_entry(15, isr15);
  idt_set_entry(16, isr16);
  idt_set_entry(17, isr17);
  idt_set_entry(18, isr18);
  idt_set_entry(19, isr19);
  idt_set_entry(20, isr20);
  idt_set_entry(21, isr21);
  idt_set_entry(22, isr22);
  idt_set_entry(23, isr23);
  idt_set_entry(24, isr24);
  idt_set_entry(25, isr25);
  idt_set_entry(26, isr26);
  idt_set_entry(27, isr27);
  idt_set_entry(28, isr28);
  idt_set_entry(29, isr29);
  idt_set_entry(30, isr30);
  idt_set_entry(31, isr31);

  log_message(&kernel_debug_logger, LOG_INFO, "interrupts", "Built IDT entries\n");
  log_message(&kernel_debug_logger, LOG_INFO, "interrupts", "Enabling interrupts\n");
  __asm__ volatile("sti");
  log_message(&kernel_debug_logger, LOG_INFO, "interrupts", "Interrupts enabled\n");
}

void isr_handler(InterruptFrame* frame, uint64_t interrupt_number) {
  switch (interrupt_number) {
    case 0:
      kernel_panic("Division by zero error", frame);
      break;
    case 1:
      kernel_panic("Debug exception", frame);
      break;
    case 2:
      kernel_panic("Non-maskable interrupt exception", frame);
      break;
    case 3:
      kernel_panic("Breakpoint exception", frame);
      break;
    case 4:
      kernel_panic("Overflow exception", frame);
      break;
    case 5:
      kernel_panic("Bound range exceeded exception", frame);
      break;
    case 6:
      kernel_panic("Invalid opcode exception", frame);
      break;
    case 7:
      kernel_panic("Device not available exception", frame);
      break;
    case 8:
      kernel_panic("Double fault exception", frame);
      break;
    case 9:
      kernel_panic("Coprocessor segment overrun exception", frame);
      break;
    case 10:
      kernel_panic("Invalid TSS exception", frame);
      break;
    case 11:
      kernel_panic("Segment not present exception", frame);
      break;
    case 12:
      kernel_panic("Stack fault exception", frame);
      break;
    case 13:
      kernel_panic("General protection fault exception", frame);
      break;
    case 14:
      uint64_t fault_address = read_cr2();
      uint64_t error_code = frame->error_code;

      const char* present = (error_code & 0x1) ? "present" : "not present";
      const char* write = (error_code & 0x2) ? "write" : "read";
      const char* user = (error_code & 0x4) ? "user" : "supervisor";
      const char* reserved = (error_code & 0x8) ? "reserved write" : "not reserved write";
      const char* instruction = (error_code & 0x10) ? "instruction fetch" : "not instruction fetch";

      kernel_panic_detailed(
        "Page Fault",
        frame,
        "A page fault occurred.\n"
        "Faulting address: 0x%016llx\n"
        "Error code: 0x%lX\n"
        "The fault was caused by a %s during a %s in %s mode.\n"
        "The fault %s.\n"
        "The fault %s.\n",
        fault_address,
        error_code,
        write,
        user,
        present,
        reserved,
        instruction
      );

      break;
    case 16:
      kernel_panic("x87 Floating point exception", frame);
      break;
    case 17:
      kernel_panic("Alignment check exception", frame);
      break;
    case 18:
      kernel_panic("Machine check exception", frame);
      break;
    case 19:
      kernel_panic("SIMD Floating point exception", frame);
      break;
    case 20:
      kernel_panic("Virtualization exception", frame);
      break;
    case 21:
      kernel_panic("Control protection exception", frame);
      break;
    default:
      kernel_panic("Reserved exception", frame);
      break;
  }
}
