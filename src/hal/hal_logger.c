#include <hal/serial.h>
#include <hal/hal_logger.h>

log_writer_t hal_writers[] = {serial_write_string};
logger_t hal_logger;

void hal_logger_initialize() {
  log_init(&hal_logger, hal_writers, sizeof(hal_writers) / sizeof(log_writer_t));
}
