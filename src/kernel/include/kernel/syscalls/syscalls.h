#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

/**
 * Total number of system calls. Increment each time a new system call is added.
 */
#define SYSCALL_COUNT 2

/**
 * Human-readable system call names
 */
typedef enum {
  SYSCALL_READ = 0,
  SYSCALL_WRITE = 1,
} SystemCallNumber;

/**
 * System call error codes
 */
typedef enum {
  SYSCALL_SUCCESS = 0,
  SYSCALL_ERROR_INVALID_SYSCALL = -1,
  SYSCALL_ERROR_INVALID_ARGS = -2,
  SYSCALL_ERROR_PERMISSION_DENIED = -3,
  SYSCALL_ERROR_NOT_IMPLEMENTED = -4,
} SystemCallError;

/**
 * Syscall argument structure. System calls support up to 6 arguments according
  * to the x86-64 calling convention.
 */
typedef struct {
  uint64_t args[6];
} SystemCallArgs;

/**
 * Possible system call argument types
 */
typedef enum {
  ARG_INT,
  ARG_UINT,
  ARG_PTR,
  ARG_SIZE,
} SyscallArgType;

/**
 * Allows system calls to return a value and an error code simultaneously.
 */
typedef struct {
  int64_t value;
  SystemCallError error;
} SystemCallReturn;

/**
 * System call handler function pointer type
 *
 * @param args The arguments passed to the system call
 * @return The return value of the system call
 */
 typedef SystemCallReturn (*SystemCallHandler)(SystemCallArgs*);

/**
 * Represents a single entry in the syscall table
 */
typedef struct {
  SystemCallHandler handler;
  const char* name;
  uint8_t num_args;
} SystemCallEntry;

/**
 * Validates the types of arguments passed to a system call
 */
bool syscall_validate_args(const SystemCallArgs* args, int num_args, ...);

/**
 * Dispatches a system call to the appropriate handler
 *
 * @param syscall_number The system call number
 * @param args The arguments passed to the system call
 * @return The return value of the system call
 */
__attribute__((hot)) SystemCallReturn syscall_handler(SystemCallNumber syscall_number, SystemCallArgs* args);

/**
 * Assembly function that calls the syscall_handler function once the syscall
 * instruction is called
 */
__attribute__((naked)) void syscall_entry();

/**
 * Initializes the MSRs and the syscall instruction
 */
void syscalls_initialize();

/**
 * Macro for defining a system call handler
 */
#define DEFINE_SYSCALL(name, num_args, ...) \
    SystemCallReturn syscall_##name(SystemCallArgs* args)

/*
 * ============================================================================
 * System Call Definitions
 * ============================================================================
 */

/**
 * Reads data from a file descriptor
 *
 * @param fd The file descriptor to read from
 * @param buf The buffer to read into
 * @param count The number of bytes to read
 * @return The number of bytes read, or -1 on error
 */
DEFINE_SYSCALL(read, 3, ARG_INT, ARG_PTR, ARG_SIZE);


/**
 * Writes data from to file descriptor
 *
 * @param fd The file descriptor to write to
 * @param buf The buffer to write from
 * @param count The number of bytes to write
 * @return The number of bytes written, or -1 on error
 */
DEFINE_SYSCALL(write, 3, ARG_INT, ARG_PTR, ARG_SIZE);
