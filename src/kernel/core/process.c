#include <kernel/process.h>
#include <kernel/debug.h>

void jump_to_usermode(void* user_function, void* user_stack) {
  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_INFO,
      "Jumping to usermode {entry=%p, user_stack=%p}\n",
      user_function,
      user_stack
    );
  }

  // asm volatile (
  //   "mov $0x23, %%ax\n"  // User data segment
  //   "mov %%ax, %%ds\n"
  //   "mov %%ax, %%es\n"
  //   "mov %%ax, %%fs\n"
  //   "mov %%ax, %%gs\n"
  //   "mov %0, %%rcx\n"    // RIP for sysretq (address to return to)
  //   "mov %1, %%rsp\n"    // Set up the user stack
  //   "mov $0x202, %%r11\n" // RFLAGS for sysretq
  //   "sysretq\n"
  //   :
  //   : "r" (user_function), "r" (user_stack)
  //   : "rax", "rcx", "r11"
  // );

  asm volatile (
    "mov %0, %%rcx\n"      // RIP for sysretq (address to return to)
    "mov %1, %%rsp\n"      // Set up the user stack
    "mov $0x202, %%r11\n"  // RFLAGS for sysretq
    "sysretq\n"
    :
    : "r" (user_function), "r" (user_stack)
    : "rcx", "r11"
  );
}
