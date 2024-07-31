#pragma once

#include <kernel/interrupts.h>

/**
 * Kernel panic, another L for da OG :(
 *
 * @param error_message The error message to display
* @param frame The interrupt frame at the time of the panic
*/
void kernel_panic(const char* error_message, InterruptFrame* frame);

/**
 * H
*/
void hcf();
