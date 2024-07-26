#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

/**
 * Defines possible log levels.
*/
typedef enum {
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR
} LogLevel;

/**
 * Abstracts a log writer, allowing the logger to write to different
 * destinations depending on implementation.
*/
typedef int (*LogWriter)(
  const char* message,
  size_t length,
  void* context
);

/**
 * Stores logger configuration
*/
typedef struct {
  LogWriter writer;
  void* writer_context;
} LoggerConfig;

/**
 * Initializes the logger with the given configuration.
 *
 * @param config The logger configuration.
*/
void logger_initialize(const LoggerConfig* config);

/**
 * Logs a message with the given log level.
 *
 * @param level The log level.
 * @param format The message format.
 * @param ... The message arguments.
*/
void logger_log(LogLevel level, const char* format, ...);
