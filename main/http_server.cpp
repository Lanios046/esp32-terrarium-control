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
    
    char offset_buf[16] = {0};
    if (httpd_query_key_value(req->uri, "offset", offset_buf, sizeof(offset_buf)) == ESP_OK) {
        offset = (size_t)atoi(offset_buf);
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

extern const char logs_html_start[] asm("_binary_logs_html_start");
extern const char logs_html_end[] asm("_binary_logs_html_end");

static esp_err_t index_handler(httpd_req_t *req) {
    size_t logs_html_len = logs_html_end - logs_html_start;
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, logs_html_start, logs_html_len);
    return ESP_OK;
}

void http_server_start() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;
    config.stack_size = 32768;
    
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

