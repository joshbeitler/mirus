#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <libk/string.h>
#include <printf/printf.h>
#include <jems/jems.h>
#include <logger.h>

#define JEMS_MAX_LEVEL 5  // Adjust this based on expected maximum nesting level

typedef enum {
  LOG_STREAM_START,
  LOG_STREAM_DATA,
  LOG_STREAM_END
} log_stream_state_t;

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
}

static void jems_writer(char ch, uintptr_t arg) {
  char **buf = (char **)arg;
  **buf = ch;
  (*buf)++;
}

static void create_json_log(
  jems_t *jems,
  log_level_t level,
  const char *component,
  const char *message,
  bool include_data
) {
  jems_object_open(jems);
  jems_key_string(jems, "level", log_level_to_string(level));
  jems_key_string(jems, "component", component);
  jems_key_string(jems, "message", message);
  if (include_data) {
    jems_key_literal(jems, "data", "", 0);  // Empty data field to be filled later
  }
  jems_object_close(jems);
}

void log_message(
  logger_t *logger,
  log_level_t level,
  const char *component,
  const char *format,
  ...
) {
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
  char *json_ptr = json_output;
  static jems_level_t jems_levels[JEMS_MAX_LEVEL];
  jems_t jems;
  jems_init(&jems, jems_levels, JEMS_MAX_LEVEL, jems_writer, (uintptr_t)&json_ptr);

  create_json_log(&jems, level, component, message, false);

  *json_ptr = '\n';
  *(json_ptr + 1) = '\0';

  for (int i = 0; i < logger->num_writers; i++) {
    logger->writers[i](json_output);
  }
}

void log_stream_start(
  logger_t *logger,
  log_level_t level,
  const char *component,
  const char *message
) {
  char json_output[MAX_LOG_MESSAGE_SIZE];
  char *json_ptr = json_output;
  static jems_level_t jems_levels[JEMS_MAX_LEVEL];
  jems_t jems;
  jems_init(&jems, jems_levels, JEMS_MAX_LEVEL, jems_writer, (uintptr_t)&json_ptr);

  jems_object_open(&jems);
  jems_key_string(&jems, "level", log_level_to_string(level));
  jems_key_string(&jems, "component", component);
  jems_key_string(&jems, "message", message);
  jems_key_literal(&jems, "data", "", 0);  // Empty data field to be filled later

  // Don't close the object yet
  *json_ptr = '\0';

  for (int i = 0; i < logger->num_writers; i++) {
    logger->writers[i](json_output);
  }
}

void log_stream_data(
  logger_t *logger,
  const char *data,
  size_t length
) {
  for (int i = 0; i < logger->num_writers; i++) {
    logger->writers[i](data);
  }
}

void log_stream_end(logger_t *logger) {
  char json_output[MAX_LOG_MESSAGE_SIZE];
  char *json_ptr = json_output;
  static jems_level_t jems_levels[JEMS_MAX_LEVEL];
  jems_t jems;
  jems_init(&jems, jems_levels, JEMS_MAX_LEVEL, jems_writer, (uintptr_t)&json_ptr);

  jems_object_close(&jems);

  *json_ptr = '\n';
  *(json_ptr + 1) = '\0';

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
  // Start the log stream
  log_stream_start(logger, level, component, message);

  // Stream the json_data in chunks
  const char *data_ptr = json_data;
  size_t remaining = strlen(json_data);
  while (remaining > 0) {
    size_t chunk_size = (remaining > 900) ? 900 : remaining;
    log_stream_data(logger, data_ptr, chunk_size);
    data_ptr += chunk_size;
    remaining -= chunk_size;
  }

  // End the log stream
  log_stream_end(logger);
}
