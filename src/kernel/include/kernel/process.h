#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * Switches to usermode by setting up a new stack and jumping to the user
 * function in ring 3 using the sysretq instruction.
 *
 * NOTE: This function does not return.
 * NOTE: This function assumes that the MSRs and syscall instructions have been
 *       already set up by the system call initialization code.
 */
void jump_to_usermode(void* user_function, void* user_stack);
