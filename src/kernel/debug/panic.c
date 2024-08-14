#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <hal/serial.h>
#include <libk/string.h>
#include <printf/printf.h>

#include <kernel/debug.h>
#include <kernel/interrupts.h>
#include <kernel/panic.h>

#include <drivers/terminal.h>

// TODO: emit a detailed log with log_message and jems

void kernel_panic(const char *error_message, InterruptFrame *frame) {
	log_message(
		&kernel_debug_logger, LOG_FATAL, "kernel", "Kernel panic encountered\n"
	);

	terminal_clear();

	printf_("Kernel panic!\n");
	printf_("%s\n\n", error_message);

	print_interrupt_frame(frame);

	printf_("\nHalting and catching fire\n");

	hcf();
}

void kernel_panic_detailed(
	const char *error_message, InterruptFrame *frame, const char *format, ...
) {
	log_message(
		&kernel_debug_logger, LOG_FATAL, "kernel", "Kernel panic encountered\n"
	);

	terminal_clear();

	printf_("Kernel panic!\n");
	printf_("%s\n\n", error_message);

	print_interrupt_frame(frame);

	// Print additional details
	printf_("\nAdditional Details:\n");
	va_list args;
	va_start(args, format);
	vprintf_(format, args);
	va_end(args);

	printf_("\nHalting and catching fire\n");

	hcf();
}

void print_interrupt_frame(InterruptFrame *frame) {
	printf_("RIP:     0x%lX\n", frame->rip);
	printf_("CS:      0x%lX\n", frame->cs);
	printf_("RFLAGS:  0x%lX\n", frame->rflags);
	printf_("RSP:     0x%lX\n", frame->rsp);
	printf_("SS:      0x%lX\n", frame->ss);

	printf_("RAX:     0x%lX\n", frame->rax);
	printf_("RBX:     0x%lX\n", frame->rbx);
	printf_("RCX:     0x%lX\n", frame->rcx);
	printf_("RDX:     0x%lX\n", frame->rdx);
	printf_("RSI:     0x%lX\n", frame->rsi);
	printf_("RDI:     0x%lX\n", frame->rdi);
	printf_("RBP:     0x%lX\n", frame->rbp);
	printf_("R8:      0x%lX\n", frame->r8);
	printf_("R9:      0x%lX\n", frame->r9);
	printf_("R10:     0x%lX\n", frame->r10);
	printf_("R11:     0x%lX\n", frame->r11);
	printf_("R12:     0x%lX\n", frame->r12);
	printf_("R13:     0x%lX\n", frame->r13);
	printf_("R14:     0x%lX\n", frame->r14);
	printf_("R15:     0x%lX\n", frame->r15);

	printf_("Interrupt Number: %lu\n", frame->interrupt_number);
	printf_("Error Code:       0x%lX\n", frame->error_code);
}

void hcf() {
	__asm__ volatile("cli");
	for (;;) {
		__asm__ volatile("hlt");
	}
}
