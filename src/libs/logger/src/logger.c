#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "logger.h"

void log_init(logger_t *logger, log_writer_t *writers, int num_writers, log_level_t level) {
    logger->writers = writers;
    logger->num_writers = num_writers;
    logger->level = level;
}


static void write_number(log_writer_t writer, uint64_t num, int base, int is_signed, int uppercase) {
    char buffer[65];
    char *ptr = buffer + sizeof(buffer) - 1;
    *ptr = '\0';

    if (is_signed && (int64_t)num < 0) {
        num = -num;
        writer("-");
    }

    do {
        uint64_t digit = num % base;
        if (digit < 10) {
            *--ptr = '0' + digit;
        } else {
            *--ptr = (uppercase ? 'A' : 'a') + (digit - 10);
        }
        num /= base;
    } while (num);

    writer(ptr);
}


static void mini_vprintf(log_writer_t writer, const char *format, va_list args) {
    const char *p = format;
    char buffer[2];  // Buffer for single characters

    while (*p) {
        if (*p == '%') {
            p++;  // Skip the '%'
            int uppercase = 0;  // Flag for uppercase hexadecimal
            switch (*p) {
                case 's': {
                    char *str = va_arg(args, char *);
                    writer(str);
                    break;
                }
                case 'd': {  // Signed decimal
                    int num = va_arg(args, int);
                    write_number(writer, num, 10, 1, 0);
                    break;
                }
                case 'u': {  // Unsigned decimal
                    uint32_t num = va_arg(args, uint32_t);
                    write_number(writer, num, 10, 0, 0);
                    break;
                }
                case 'l':
                    p++;
                    switch (*p) {
                        case 'u': {  // Unsigned long decimal
                            uint64_t num = va_arg(args, uint64_t);
                            write_number(writer, num, 10, 0, 0);
                            break;
                        }
                        case 'X': {  // Uppercase hexadecimal for uint64_t
                            uint64_t num = va_arg(args, uint64_t);
                            uppercase = 1;
                            write_number(writer, num, 16, 0, uppercase);
                            break;
                        }
                    }
                    break;
                case 'x': {  // Lowercase hexadecimal for uint32_t
                    uint32_t num = va_arg(args, uint32_t);
                    write_number(writer, num, 16, 0, 0);
                    break;
                }
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
