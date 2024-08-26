#pragma once

#include <stdint.h>
#include <stddef.h>

#include <logger.h>

extern logger_t kernel_debug_logger;

#define JEMS_MAX_LEVEL 10

void debug_logger_initialize();

void debug_test_syscalls();
void debug_test_exceptions();
void debug_test_buddy_allocator();

static inline void jems_writer(char ch, uintptr_t arg) {
	logger_t *logger = (logger_t *)arg;
	char str[2] = {ch, '\0'};
	log_stream_data(logger, str, 1);
}
