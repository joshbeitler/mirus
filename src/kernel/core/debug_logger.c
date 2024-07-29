// log_writer_t writers[] = {terminal_write_string, serial_write_string};

//    log_init(&logger, writers, 2, LOG_INFO);
//    log_message(&logger, LOG_DEBUG, "This is a debug message: %s", "not shown");
//    log_message(&logger, LOG_INFO, "Info level log: %s", "shown");
//
#include <kernel/debug_logger.h>
#include <kernel/serial.h>

log_writer_t writers[] = {serial_write_string};
logger_t kernel_debug_logger;

void debug_logger_initialize() {
  log_init(&kernel_debug_logger, writers, sizeof(writers) / sizeof(log_writer_t), LOG_INFO);
}
