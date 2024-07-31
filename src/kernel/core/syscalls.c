#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#include <printf/printf.h>
#include <libk/string.h>

#include <kernel/syscalls.h>
#include <kernel/debug.h>

/*
 * ============================================================================
 * System call definitions
 * ============================================================================
 */

// Dummy write implementation
DEFINE_SYSCALL(read, 3, ARG_INT, ARG_PTR, ARG_SIZE) {
  if (!syscall_validate_args(args, 3, ARG_INT, ARG_PTR, ARG_SIZE)) {
    return (SystemCallReturn){.value = 0, .error = SYSCALL_ERROR_INVALID_ARGS};
  }

  int fd = (int)args->args[0];
  void* buf = (void*)args->args[1];
  size_t count = (size_t)args->args[2];

  // Implement read syscall
  // For now, just return the count
  return (SystemCallReturn){.value = count, .error = SYSCALL_SUCCESS};
}

// Dummy read implementation
DEFINE_SYSCALL(write, 3, ARG_INT, ARG_PTR, ARG_SIZE) {
  if (!syscall_validate_args(args, 3, ARG_INT, ARG_PTR, ARG_SIZE)) {
    return (SystemCallReturn){.value = 0, .error = SYSCALL_ERROR_INVALID_ARGS};
  }

  int fd = (int)args->args[0];
  const void* buf = (const void*)args->args[1];
  size_t count = (size_t)args->args[2];

  // Implement write syscall
  // For now, just return the count
  return (SystemCallReturn){.value = count, .error = SYSCALL_SUCCESS};
}

__attribute__((aligned(64)))
static const SystemCallEntry syscall_table[SYSCALL_COUNT] = {
  [SYSCALL_READ]  = {syscall_read,  "read",  3},
  [SYSCALL_WRITE] = {syscall_write, "write", 3},
  // Add other syscalls here
};

/*
 * ============================================================================
 * System call utility functions
 * ============================================================================
 */

// TODO: Not thread safe due to static buffer
// TODO: Treats all argument types as hexedecimal. Add special conditions for
//       other types.
static char* format_syscall_args(const SystemCallArgs* args, int num_args) {
  static char arg_str[256];
  memset(arg_str, 0, sizeof(arg_str));

  int offset = 0;

  for (int i = 0; i < num_args && i < 6; i++) {
    offset += snprintf_(
      arg_str + offset,
      sizeof(arg_str) - offset,
      "arg%d=0x%llx",
      i + 1,
      args->args[i]
    );

    if (i < num_args - 1 && i < 5) {
      offset += snprintf_(arg_str + offset, sizeof(arg_str) - offset, ", ");
    }
  }

  return arg_str;
}

bool syscall_validate_args(const SystemCallArgs* args, int num_args, ...) {
  va_list ap;
  va_start(ap, num_args);
  bool valid = true;
  for (int i = 0; i < num_args && valid; i++) {
    SystemCallArgType type = va_arg(ap, SystemCallArgType);
    switch (type) {
      case ARG_INT:
      case ARG_UINT:
      case ARG_PTR:
        break;
      case ARG_SIZE:
        valid = args->args[i] > 0;
        break;
      default:
        valid = false;
    }
  }
  va_end(ap);
  return valid;
}

SystemCallReturn syscall_handler(
  SystemCallNumber syscall_number,
  SystemCallArgs* args
) {
  SystemCallReturn result = {0};

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_DEBUG,
      "System call entered: {number=%d}\n",
      syscall_number
    );
  }

  if (__builtin_expect(syscall_number >= SYSCALL_COUNT, 0) ||
    __builtin_expect(syscall_table[syscall_number].handler == NULL, 0)) {
    if (DEBUG) {
      log_message(
        &kernel_debug_logger,
        LOG_ERROR,
        "  Invalid system call: No system call found\n",
        syscall_number
      );
    }

    return (SystemCallReturn){.value = 0, .error = SYSCALL_ERROR_INVALID_SYSCALL};
  }

  const SystemCallEntry* entry = &syscall_table[syscall_number];
  char* formatted_args = format_syscall_args(args, entry->num_args);

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_DEBUG,
      "  Dispatching system call: {name=%s, number=%d, args={%s}}\n",
      syscall_table[syscall_number].name,
      syscall_number,
      formatted_args
    );
  }

  // Validate number of arguments
  if (syscall_table[syscall_number].num_args > 6) {
    if (DEBUG) {
      log_message(
        &kernel_debug_logger,
        LOG_ERROR,
        "  Invalid system call: Too many arguments\n"
      );
    }

    return (SystemCallReturn){.value = 0, .error = SYSCALL_ERROR_INVALID_ARGS};
  }

  result = entry->handler(args);

  if (DEBUG) {
    log_message(
      &kernel_debug_logger,
      LOG_DEBUG,
      "  System call completed: {name=%s, return=%lld, error=%d}\n",
      syscall_table[syscall_number].name,
      result.value,
      result.error
    );
  }

  return result;
}

__attribute__((naked)) void syscall_entry() {
  asm volatile (
    "push %%rcx\n"           // Save user RIP
    "push %%r11\n"           // Save user RFLAGS
    "push %%rax\n"           // Save syscall number
    "push %%rdi\n"           // Save arg1
    "push %%rsi\n"           // Save arg2
    "push %%rdx\n"           // Save arg3
    "push %%r10\n"           // Save arg4
    "push %%r8\n"            // Save arg5
    "push %%r9\n"            // Save arg6
    "mov %%rsp, %%rdi\n"     // Pass stack pointer as first argument
    "call syscall_handler\n"
    "add $48, %%rsp\n"       // Adjust stack pointer (6 args * 8 bytes)
    "pop %%r11\n"            // Restore user RFLAGS
    "pop %%rcx\n"            // Restore user RIP
    "sysretq\n"
    : : : "memory"
  );
}

void syscalls_initialize() {
  // Set up MSRs for syscall
  log_message(&kernel_debug_logger, LOG_INFO, "  Enabling syscall MSRs\n");

  uint64_t star = (uint64_t)0x0013000800000000ULL;
  uint64_t lstar = (uint64_t)syscall_entry;
  uint64_t sfmask = (uint64_t)0x200;

  asm volatile ("wrmsr" : : "c" (0xC0000081), "a" ((uint32_t)star), "d" ((uint32_t)(star >> 32)));
  asm volatile ("wrmsr" : : "c" (0xC0000082), "a" ((uint32_t)lstar), "d" ((uint32_t)(lstar >> 32)));
  asm volatile ("wrmsr" : : "c" (0xC0000084), "a" ((uint32_t)sfmask), "d" ((uint32_t)(sfmask >> 32)));

  log_message(&kernel_debug_logger, LOG_INFO, "  Enabled syscall MSRs\n");

  // Enable syscall instruction
  log_message(&kernel_debug_logger, LOG_INFO, "  Enabling syscall instruction in EFER\n");
  uint64_t efer;
  asm volatile ("rdmsr" : "=A" (efer) : "c" (0xC0000080));
  efer |= 1ULL << 0;  // Set SCE (System Call Enable) bit
  asm volatile ("wrmsr" : : "c" (0xC0000080), "A" (efer));

  log_message(&kernel_debug_logger, LOG_INFO, "  Enabled syscall instruction\n");

  // Prefetch syscall table
  log_message(&kernel_debug_logger, LOG_INFO, "  Prefetching system call table\n");
  __builtin_prefetch(syscall_table, 0, 3);
  log_message(&kernel_debug_logger, LOG_INFO, "  System call table prefetched\n");
}
