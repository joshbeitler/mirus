#pragma once

#include <stdarg.h>  // Include standard argument macros

// Define log levels
typedef enum {
    LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG
} log_level_t;

typedef void (*log_writer_t)(const char*);

// Logger structure now includes log level and an array of writers
typedef struct {
    log_writer_t *writers;  // Array of writer functions
    int num_writers;        // Number of writers
    log_level_t level;      // Current logging level
} logger_t;

// Logger API
void log_init(logger_t *logger, log_writer_t *writers, int num_writers, log_level_t level);
void log_message(logger_t *logger, log_level_t level, const char *format, ...);
