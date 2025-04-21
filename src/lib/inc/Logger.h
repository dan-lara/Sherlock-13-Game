#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define LOG_FILE_PATH "system.log"

typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_DEBUG
} LogLevel;


static FILE *log_file = NULL;

void init_logger(const char *file_path, const int append);
void log_message(LogLevel level, const char *format, ...);
void close_logger();


#endif // Logger.h