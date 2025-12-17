#include "http_server.h"
#include "logs.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include <string.h>
#include <stdio.h>

static const char* HTTP_TAG = "HTTP_SERVER";
static httpd_handle_t server_handle = NULL;

static esp_err_t logs_handler(httpd_req_t *req) {
    char response[LOG_BUFFER_SIZE];
    size_t offset = 0;
    
    const char* offset_str = httpd_query_key_value(req->uri, "offset");
    if (offset_str != NULL) {
        offset = (size_t)atoi(offset_str);
    }
    
    size_t len = logs_get(response, sizeof(response), offset);
    
    httpd_resp_set_type(req, "text/plain; charset=utf-8");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, response, len);
    
    return ESP_OK;
}

static esp_err_t logs_count_handler(httpd_req_t *req) {
    size_t count = logs_get_count();
    char response[32];
    snprintf(response, sizeof(response), "{\"count\":%zu}", count);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, response, strlen(response));
    
    return ESP_OK;
}

static esp_err_t logs_clear_handler(httpd_req_t *req) {
    logs_clear();
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    
    return ESP_OK;
}

static const char logs_html[] = R"(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Логи ESP32</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: 'Courier New', monospace; background: #1e1e1e; color: #d4d4d4; padding: 20px; }
        .container { max-width: 1200px; margin: 0 auto; }
        .header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; padding-bottom: 10px; border-bottom: 1px solid #3e3e3e; }
        h1 { font-size: 24px; color: #ffffff; }
        .controls { display: flex; gap: 10px; }
        button { padding: 8px 16px; background: #0e639c; color: white; border: none; border-radius: 4px; cursor: pointer; font-size: 14px; }
        button:hover { background: #1177bb; }
        button:active { background: #0a4d73; }
        .auto-scroll { display: flex; align-items: center; gap: 8px; }
        input[type="checkbox"] { width: 18px; height: 18px; cursor: pointer; }
        .log-container { background: #252526; border: 1px solid #3e3e3e; border-radius: 4px; padding: 15px; height: calc(100vh - 150px); overflow-y: auto; font-size: 13px; line-height: 1.6; }
        .log-entry { margin-bottom: 4px; word-wrap: break-word; }
        .log-entry.error { color: #f48771; }
        .log-entry.warn { color: #dcdcaa; }
        .log-entry.info { color: #4ec9b0; }
        .log-entry.debug { color: #9cdcfe; }
        .status { margin-top: 10px; padding: 8px; background: #2d2d30; border-radius: 4px; font-size: 12px; color: #858585; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Логи ESP32</h1>
            <div class="controls">
                <div class="auto-scroll">
                    <input type="checkbox" id="autoScroll" checked>
                    <label for="autoScroll">Автопрокрутка</label>
                </div>
                <button onclick="clearLogs()">Очистить</button>
                <button onclick="refreshLogs()">Обновить</button>
            </div>
        </div>
        <div class="log-container" id="logContainer"></div>
        <div class="status" id="status">Загрузка...</div>
    </div>
    <script>
        let logOffset = 0;
        let autoScrollEnabled = true;
        let refreshInterval = null;
        const logContainer = document.getElementById('logContainer');
        const statusDiv = document.getElementById('status');
        const autoScrollCheckbox = document.getElementById('autoScroll');
        autoScrollCheckbox.addEventListener('change', (e) => { autoScrollEnabled = e.target.checked; if (autoScrollEnabled) scrollToBottom(); });
        function scrollToBottom() { logContainer.scrollTop = logContainer.scrollHeight; }
        function getLogLevelClass(level) { switch(level) { case 'E': return 'error'; case 'W': return 'warn'; case 'I': return 'info'; case 'D': return 'debug'; default: return ''; } }
        async function fetchLogs() {
            try {
                const response = await fetch('/api/logs?offset=' + logOffset);
                if (!response.ok) throw new Error('Ошибка загрузки логов');
                const text = await response.text();
                if (text.trim().length === 0) return;
                const lines = text.split('\n').filter(line => line.trim().length > 0);
                lines.forEach(line => {
                    const entry = document.createElement('div');
                    entry.className = 'log-entry';
                    const match = line.match(/^\[(\d{2}:\d{2}:\d{2})\] \[([EWID])\] \[([^\]]+)\] (.+)$/);
                    if (match) { const [, time, level, tag, message] = match; entry.className += ' ' + getLogLevelClass(level); entry.textContent = line; } else { entry.textContent = line; }
                    logContainer.appendChild(entry);
                });
                logOffset += lines.length;
                if (autoScrollEnabled) setTimeout(scrollToBottom, 10);
                statusDiv.textContent = 'Загружено записей: ' + logOffset;
            } catch (error) { statusDiv.textContent = 'Ошибка: ' + error.message; }
        }
        async function getLogCount() { try { const response = await fetch('/api/logs/count'); const data = await response.json(); return data.count || 0; } catch (error) { return 0; } }
        async function clearLogs() {
            if (!confirm('Очистить все логи?')) return;
            try {
                const response = await fetch('/api/logs/clear', { method: 'POST' });
                if (response.ok) { logContainer.innerHTML = ''; logOffset = 0; statusDiv.textContent = 'Логи очищены'; }
            } catch (error) { statusDiv.textContent = 'Ошибка: ' + error.message; }
        }
        function refreshLogs() { fetchLogs(); }
        function startAutoRefresh() {
            refreshInterval = setInterval(async () => {
                const currentCount = await getLogCount();
                if (currentCount > logOffset) await fetchLogs();
            }, 1000);
        }
        function stopAutoRefresh() { if (refreshInterval) { clearInterval(refreshInterval); refreshInterval = null; } }
        window.addEventListener('beforeunload', () => { stopAutoRefresh(); });
        fetchLogs();
        startAutoRefresh();
    </script>
</body>
</html>
)";

static esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, logs_html, strlen(logs_html));
    return ESP_OK;
}

void http_server_start() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;
    
    ESP_LOGI(HTTP_TAG, "Starting HTTP server on port: '%d'", config.server_port);
    
    if (httpd_start(&server_handle, &config) == ESP_OK) {
        httpd_uri_t logs_uri = {
            .uri       = "/api/logs",
            .method    = HTTP_GET,
            .handler   = logs_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server_handle, &logs_uri);
        
        httpd_uri_t logs_count_uri = {
            .uri       = "/api/logs/count",
            .method    = HTTP_GET,
            .handler   = logs_count_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server_handle, &logs_count_uri);
        
        httpd_uri_t logs_clear_uri = {
            .uri       = "/api/logs/clear",
            .method    = HTTP_POST,
            .handler   = logs_clear_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server_handle, &logs_clear_uri);
        
        httpd_uri_t index_uri = {
            .uri       = "/logs",
            .method    = HTTP_GET,
            .handler   = index_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server_handle, &index_uri);
        
        ESP_LOGI(HTTP_TAG, "HTTP server started");
    } else {
        ESP_LOGE(HTTP_TAG, "Error starting HTTP server");
    }
}

void http_server_stop() {
    if (server_handle != NULL) {
        httpd_stop(server_handle);
        server_handle = NULL;
        ESP_LOGI(HTTP_TAG, "HTTP server stopped");
    }
}

