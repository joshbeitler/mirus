#include <kernel/debug.h>
#include <kernel/process.h>

// TODO: use jemi for logging

void jump_to_usermode(void *user_function, void *user_stack) {
	if (DEBUG) {
		log_message(
			&kernel_debug_logger,
			LOG_INFO,
			"process",
			"Jumping to usermode {entry=%p, user_stack=%p}\n",
			user_function,
			user_stack
		);
	}

	asm volatile("mov %0, %%rcx\n"	   // RIP for sysretq (address to return to)
				 "mov %1, %%rsp\n"	   // Set up the user stack
				 "mov $0x202, %%r11\n" // RFLAGS for sysretq
				 "sysretq\n"
				 :
				 : "r"(user_function), "r"(user_stack)
				 : "rcx", "r11");
}
