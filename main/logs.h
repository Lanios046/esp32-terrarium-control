#ifndef LOGS_H
#define LOGS_H

#include <stddef.h>
#include <stdint.h>

#define LOG_BUFFER_SIZE 8192
#define MAX_LOG_ENTRIES 200

struct LogEntry {
    char tag[16];
    char level;
    char message[128];
    uint32_t timestamp;
};

void logs_init();
void logs_add(const char* tag, char level, const char* message);
size_t logs_get(char* buffer, size_t buffer_size, size_t offset);
size_t logs_get_count();
void logs_clear();

#endif // LOGS_H

