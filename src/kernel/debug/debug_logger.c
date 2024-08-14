#include <hal/serial.h>

#include <kernel/debug.h>

log_writer_t kernel_writers[] = {serial_write_string};
logger_t kernel_debug_logger;

void debug_logger_initialize() {
	log_init(
		&kernel_debug_logger,
		kernel_writers,
		sizeof(kernel_writers) / sizeof(log_writer_t)
	);
}
