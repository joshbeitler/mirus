#include "logger.h"

void log_init(logger_t *logger, log_writer_t *writers, int num_writers, log_level_t level) {
    logger->writers = writers;
    logger->num_writers = num_writers;
    logger->level = level;
}

static void mini_vprintf(log_writer_t writer, const char *format, va_list args) {
    // Simplified implementation for demonstration
    // You would expand this to properly format all required types
    const char *p = format;
    char buffer[256];  // Simple buffer for output

    while (*p) {
        if (*p == '%') {
            p++;  // Skip the '%'
            switch (*p) {
                case 's': {
                    char *str = va_arg(args, char *);
                    writer(str);
                    break;
                }
                // Add cases for other specifiers as needed
            }
            p++;  // Skip the format specifier
        } else {
            // Output a single character at a time
            buffer[0] = *p++;
            buffer[1] = '\0';
            writer(buffer);
        }
    }
}

void log_message(logger_t *logger, log_level_t level, const char *format, ...) {
    if (level <= logger->level) {
        va_list args;
        va_start(args, format);
        for (int i = 0; i < logger->num_writers; i++) {
            mini_vprintf(logger->writers[i], format, args);
        }
        va_end(args);
    }
}
