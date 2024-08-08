#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <libk/string.h>
#include <printf/printf.h>
#include <jems/jems.h>

#include <logger.h>

#define JEMS_MAX_LEVEL 5  // Adjust this based on expected maximum nesting level

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
  const char *json_data
) {
  jems_object_open(jems);

  jems_key_string(jems, "level", log_level_to_string(level));
  jems_key_string(jems, "component", component);
  jems_key_string(jems, "message", message);

  if (json_data) {
    jems_key_literal(jems, "data", json_data, strlen(json_data));
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

  create_json_log(&jems, level, component, message, NULL);

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
  // Copy message to cleaned_message, removing trailing newline if present
  char cleaned_message[MAX_LOG_MESSAGE_SIZE];
  size_t i;
  for (i = 0; i < MAX_LOG_MESSAGE_SIZE - 1 && message[i] != '\0'; i++) {
    cleaned_message[i] = message[i];
  }
  cleaned_message[i] = '\0';

  // Remove trailing newline if present
  if (i > 0 && cleaned_message[i - 1] == '\n') {
    cleaned_message[i - 1] = '\0';
  }

  char json_output[MAX_LOG_MESSAGE_SIZE];
  char *json_ptr = json_output;

  static jems_level_t jems_levels[JEMS_MAX_LEVEL];
  jems_t jems;
  jems_init(&jems, jems_levels, JEMS_MAX_LEVEL, jems_writer, (uintptr_t)&json_ptr);

  create_json_log(&jems, level, component, cleaned_message, json_data);

  *json_ptr = '\n';
  *(json_ptr + 1) = '\0';

  for (int i = 0; i < logger->num_writers; i++) {
    logger->writers[i](json_output);
  }
}
