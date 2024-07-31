#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include <printf/printf.h>
#include "logger.h"

#define MAX_LOG_MESSAGE_SIZE 1024  // Adjust the size as needed

const char* get_log_level_string(log_level_t level) {
  switch (level) {
    case LOG_DEBUG:   return "[DEBUG]   ";
    case LOG_INFO:    return "[INFO]    ";
    case LOG_WARNING: return "[WARNING] ";
    case LOG_ERROR:   return "[ERROR]   ";
    case LOG_FATAL:   return "[FATAL]   ";
    default:          return "[UNKNOWN] ";
  }
}

void log_init(logger_t *logger, log_writer_t *writers, int num_writers) {
  logger->writers = writers;
  logger->num_writers = num_writers;
}

void log_message(logger_t *logger, log_level_t level, const char *format, ...) {
  char buffer[MAX_LOG_MESSAGE_SIZE];
  va_list args;

  // Prepare complete format with log level
  char complete_format[MAX_LOG_MESSAGE_SIZE];
  snprintf_(complete_format, MAX_LOG_MESSAGE_SIZE, "%s%s", get_log_level_string(level), format);

  va_start(args, format);
  // Format the string into the buffer using the complete format
  vsnprintf_(buffer, MAX_LOG_MESSAGE_SIZE, complete_format, args);

  // Output the formatted string using each writer
  for (int i = 0; i < logger->num_writers; i++) {
    logger->writers[i](buffer);
  }

  va_end(args);
}
