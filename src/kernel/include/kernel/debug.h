#pragma once

#include <logger.h>

extern logger_t kernel_debug_logger;

void debug_logger_initialize();

void debug_test_syscalls();
void debug_test_exceptions();
