
#include "../inc/Logger.h"
#include <stdio.h>
#include <stdlib.h>

static const char *LOG_LEVEL_STRINGS[] = {
    "INFO",
    "WARNING",
    "ERROR",
    "DEBUG"
};

void init_logger(const char *file_path, const int append) {
    if (append)
        log_file = fopen(file_path, "a");
    else 
        log_file = fopen(file_path, "w");
    if (!log_file) {
        fprintf(stderr, "Failed to open log file: %s\n", file_path);
        exit(EXIT_FAILURE);
    }
}

void log_message(LogLevel level, const char *format, ...) {
    if (!log_file) {
        init_logger(LOG_FILE_PATH, 0);
    }

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char time_buffer[20];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", local_time);

    fprintf(log_file, "[%s] [%s] ", time_buffer, LOG_LEVEL_STRINGS[level]);

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fprintf(log_file, "\n");
    fflush(log_file);
}

void close_logger() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

