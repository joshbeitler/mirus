#pragma once
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

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
} logger_t;

// Logger API
void log_init(logger_t *logger, log_writer_t *writers, int num_writers);
void log_message(logger_t *logger, log_level_t level, const char *component, const char *format, ...);
void log_complex(logger_t *logger, log_level_t level, const char *component, const char *message, const char *json_data);

// New streaming API
void log_stream_start(logger_t *logger, log_level_t level, const char *component, const char *message);
void log_stream_data(logger_t *logger, const char *data, size_t length);
void log_stream_end(logger_t *logger);

// Utility functions
const char* log_level_to_string(log_level_t level);





// TODO: new API entirely for streaming, based around putchar implementation as the writer
