#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <libk/string.h>
#include <printf/printf.h>

#include "logger.h"

const char* log_level_to_string(log_level_t level) {
  switch (level) {
    case LOG_DEBUG:   return "DEBUG";
    case LOG_INFO:    return "INFO";
    case LOG_WARNING: return "WARNING";
    case LOG_ERROR:   return "ERROR";
    case LOG_FATAL:   return "FATAL";
    default:          return "UNKNOWN";
  }
}

void log_init(logger_t *logger, log_writer_t *writers, int num_writers) {
  logger->writers = writers;
  logger->num_writers = num_writers;
  logger->level = LOG_INFO;  // Default log level
}

void log_message(
  logger_t *logger,
  log_level_t level,
  const char *component,
  const char *format,
  ...
) {
  if (level < logger->level) return;

  char message[MAX_LOG_MESSAGE_SIZE];
  va_list args;
  va_start(args, format);
  vsnprintf_(message, sizeof(message), format, args);
  va_end(args);

  // Remove trailing newline if present
  size_t msg_len = strlen(message);
  if (msg_len > 0 && message[msg_len - 1] == '\n') {
    message[msg_len - 1] = '\0';
  }

  char json_output[MAX_LOG_MESSAGE_SIZE];
  snprintf_(
    json_output,
    sizeof(json_output),
    "{\"level\":\"%s\",\"component\":\"%s\",\"message\":\"%s\"}\n",
    log_level_to_string(level),
    component,
    message
  );

  for (int i = 0; i < logger->num_writers; i++) {
    logger->writers[i](json_output);
  }
}

void log_complex(
  logger_t *logger,
  log_level_t level,
  const char *component,
  const char *message,
  const char *json_data
) {
  if (level < logger->level) return;

  // Remove trailing newline from message if present
  size_t msg_len = strlen(message);
  char cleaned_message[MAX_LOG_MESSAGE_SIZE];

  for (size_t i = 0; i < msg_len && i < MAX_LOG_MESSAGE_SIZE - 1; i++) {
    cleaned_message[i] = message[i];
  }

  cleaned_message[msg_len < MAX_LOG_MESSAGE_SIZE - 1 ? msg_len : MAX_LOG_MESSAGE_SIZE - 1] = '\0';

  if (msg_len > 0 && cleaned_message[msg_len - 1] == '\n') {
    cleaned_message[msg_len - 1] = '\0';
  }

  char json_output[MAX_LOG_MESSAGE_SIZE];
  snprintf_(
    json_output,
    sizeof(json_output),
    "{\"level\":\"%s\",\"component\":\"%s\",\"message\":\"%s\",\"data\":%s}\n",
    log_level_to_string(level),
    component,
    cleaned_message,
    json_data
  );

  for (int i = 0; i < logger->num_writers; i++) {
    logger->writers[i](json_output);
  }
}
