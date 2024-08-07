#pragma once

#include <stdarg.h>
#include <stdbool.h>

#define MAX_LOG_MESSAGE_SIZE 4096

// Define log levels
typedef enum {
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,
  LOG_FATAL
} log_level_t;

typedef void (*log_writer_t)(const char*);

// Logger structure
typedef struct {
  log_writer_t *writers;  // Array of writer functions
  int num_writers;        // Number of writers
  log_level_t level;      // Current logging level
} logger_t;

// Logger API
void log_init(logger_t *logger, log_writer_t *writers, int num_writers);
void log_message(logger_t *logger, log_level_t level, const char *component, const char *format, ...);
void log_complex(logger_t *logger, log_level_t level, const char *component, const char *message, const char *json_data);

// Utility functions
const char* log_level_to_string(log_level_t level);
