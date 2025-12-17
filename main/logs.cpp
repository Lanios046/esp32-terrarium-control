#include "logs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>
#include <time.h>

static LogEntry log_buffer[MAX_LOG_ENTRIES];
static size_t log_count = 0;
static size_t log_index = 0;
static SemaphoreHandle_t log_mutex = NULL;

void logs_init() {
    log_mutex = xSemaphoreCreateMutex();
    if (log_mutex == NULL) {
        ESP_LOGE("LOGS", "Failed to create mutex");
        return;
    }
    memset(log_buffer, 0, sizeof(log_buffer));
    log_count = 0;
    log_index = 0;
}

void logs_add(const char* tag, char level, const char* message) {
    if (log_mutex == NULL) return;
    
    if (xSemaphoreTake(log_mutex, portMAX_DELAY) == pdTRUE) {
        time_t now;
        time(&now);
        
        LogEntry* entry = &log_buffer[log_index];
        
        strncpy(entry->tag, tag, sizeof(entry->tag) - 1);
        entry->tag[sizeof(entry->tag) - 1] = '\0';
        entry->level = level;
        strncpy(entry->message, message, sizeof(entry->message) - 1);
        entry->message[sizeof(entry->message) - 1] = '\0';
        entry->timestamp = (uint32_t)now;
        
        log_index = (log_index + 1) % MAX_LOG_ENTRIES;
        if (log_count < MAX_LOG_ENTRIES) {
            log_count++;
        }
        
        xSemaphoreGive(log_mutex);
    }
}

size_t logs_get(char* buffer, size_t buffer_size, size_t offset) {
    if (log_mutex == NULL || buffer == NULL || buffer_size == 0) {
        return 0;
    }
    
    size_t written = 0;
    
    if (xSemaphoreTake(log_mutex, portMAX_DELAY) == pdTRUE) {
        size_t start_idx = 0;
        if (log_count == MAX_LOG_ENTRIES) {
            start_idx = log_index;
        }
        
        size_t count = log_count;
        if (offset < count) {
            count -= offset;
            start_idx = (start_idx + offset) % MAX_LOG_ENTRIES;
            
            for (size_t i = 0; i < count && written < buffer_size - 1; i++) {
                size_t idx = (start_idx + i) % MAX_LOG_ENTRIES;
                LogEntry* entry = &log_buffer[idx];
                
                struct tm timeinfo;
                localtime_r((time_t*)&entry->timestamp, &timeinfo);
                
                int len = snprintf(buffer + written, buffer_size - written,
                    "[%02d:%02d:%02d] [%c] [%s] %s\n",
                    timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                    entry->level, entry->tag, entry->message);
                
                if (len > 0 && written + len < buffer_size) {
                    written += len;
                } else {
                    break;
                }
            }
        }
        
        xSemaphoreGive(log_mutex);
    }
    
    buffer[written] = '\0';
    return written;
}

size_t logs_get_count() {
    if (log_mutex == NULL) return 0;
    
    size_t count = 0;
    if (xSemaphoreTake(log_mutex, portMAX_DELAY) == pdTRUE) {
        count = log_count;
        xSemaphoreGive(log_mutex);
    }
    return count;
}

void logs_clear() {
    if (log_mutex == NULL) return;
    
    if (xSemaphoreTake(log_mutex, portMAX_DELAY) == pdTRUE) {
        memset(log_buffer, 0, sizeof(log_buffer));
        log_count = 0;
        log_index = 0;
        xSemaphoreGive(log_mutex);
    }
}

