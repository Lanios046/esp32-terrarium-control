#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <nvs_flash.h>
#include "driver/gpio.h"

#include "bme280.h"

#include "freertos/timers.h"

#include "etl/string.h"
#include "etl/to_string.h"

#include "esp_log.h"
#include "esp_netif_sntp.h"
#include <stdarg.h>

#include "main.h"
#include "settings.h"
#include "config.h"
#include "wifi.h"
#include "logs.h"
#include "http_server.h"
#include "mdns_manager.h"

constexpr auto TIME_TAG = "TIME";
constexpr auto TIME_SERVER = "time.windows.com";
constexpr auto TIME_REQUEST_RETRY_COUNT = 10;

static bool wifiIsConnected = false;
static bool timeIsSync = false;

static i2c_bus_handle_t i2c_bus = NULL;
static bme280_handle_t bme280 = NULL;

static i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
};

TaskHandle_t networkTaskHandle = NULL;
TaskHandle_t timerTaskHandle = NULL;
TaskHandle_t restartTaskHandle = NULL;

static int (*original_vprintf)(const char*, va_list) = NULL;

static int log_vprintf(const char* fmt, va_list args) {
    char buffer[256];
    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args_copy);
    va_end(args_copy);
    
    if (len > 0 && len < sizeof(buffer)) {
        buffer[len] = '\0';
        
        const char* tag = "LOG";
        char level = 'I';
        
        if (strstr(buffer, "E (") || strstr(buffer, "ERROR")) {
            level = 'E';
        } else if (strstr(buffer, "W (") || strstr(buffer, "WARN")) {
            level = 'W';
        } else if (strstr(buffer, "I (") || strstr(buffer, "INFO")) {
            level = 'I';
        } else if (strstr(buffer, "D (") || strstr(buffer, "DEBUG")) {
            level = 'D';
        }
        
        const char* tag_start = strstr(buffer, "(");
        if (tag_start) {
            const char* tag_end = strstr(tag_start, ")");
            if (tag_end && tag_end - tag_start > 1) {
                char tag_buf[16] = {0};
                size_t tag_len = tag_end - tag_start - 1;
                if (tag_len < sizeof(tag_buf) - 1) {
                    strncpy(tag_buf, tag_start + 1, tag_len);
                    tag_buf[tag_len] = '\0';
                    tag = tag_buf;
                }
            }
        }
        
        const char* msg_start = strstr(buffer, ": ");
        if (msg_start && msg_start[2] != '\0') {
            logs_add(tag, level, msg_start + 2);
        } else {
            logs_add(tag, level, buffer);
        }
    }
    
    if (original_vprintf) {
        return original_vprintf(fmt, args);
    }
    return vprintf(fmt, args);
}

static void networkTask(void *arg)
{
    wifi_initialize();
    vTaskDelay(1000/portTICK_PERIOD_MS);
    wifi_start_scan();
    vTaskDelay(1000/portTICK_PERIOD_MS);

    while(1){
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static void handleLightAndFountain(bool isDay)
{
    if (isDay)
    {
        gpio_set_level(LIGHT_PIN, ON);
        gpio_set_level(FOUNTAIN_PIN, ON);
    }
    else
    {
        gpio_set_level(LIGHT_PIN, OFF);
        gpio_set_level(FOUNTAIN_PIN, OFF);
    }
}

static void handleTemperature(bool isDay)
{
    float temperature = 0.0f;
    if (ESP_OK == bme280_read_temperature(bme280, &temperature))
    {
        ESP_LOGI("temperature", "temperature = %f",temperature);
        
        if (isDay)
        {
            if (temperature < TEMP_DAY_MIN)
                gpio_set_level(HEAT_PIN, ON);
            else if (temperature > TEMP_DAY_MAX)
                gpio_set_level(HEAT_PIN, OFF);
        }
        else
        {
            if (temperature < TEMP_NIGHT_MIN)
                gpio_set_level(HEAT_PIN, ON);
            else if (temperature > TEMP_NIGHT_MAX)
                gpio_set_level(HEAT_PIN, OFF);
        }
    }
}

static void handleHumidity()
{
    float humidity = 0.0f;
    if (ESP_OK == bme280_read_humidity(bme280, &humidity))
    {
        ESP_LOGI("humidity", "humidity = %f",humidity);

        if (humidity < HUMIDITY_MIN)
            gpio_set_level(HUMIDER_PIN, ON);
        else if (humidity > HUMIDITY_MAX)
            gpio_set_level(HUMIDER_PIN, OFF);
    }
}

static void handleDayNightMode(bool isDay)
{
    handleLightAndFountain(isDay);
    handleTemperature(isDay);
}

static void timerTask(void *arg)
{
    while(1){
        vTaskDelay(5000/portTICK_PERIOD_MS);

        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        bool isDay = (timeinfo.tm_hour >= DAY_START_HOUR && timeinfo.tm_hour <= DAY_END_HOUR);
        handleDayNightMode(isDay);
        handleHumidity();
    }
    vTaskDelete(NULL);
}

static void restartTask(void *arg)
{
    while(1){
        vTaskDelay(30000/portTICK_PERIOD_MS);
        
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        if (timeinfo.tm_hour == RESTART_HOUR && timeinfo.tm_min == RESTART_MIN && 
            timeinfo.tm_sec > RESTART_SEC_MIN && timeinfo.tm_sec < RESTART_SEC_MAX){
            esp_restart();
        }
    }
    vTaskDelete(NULL);
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI("TIME", "Time synchronization event successful");
}

void synctime()
{
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(TIME_SERVER);
    config.start = false;
    config.server_from_dhcp = false;
    config.sync_cb = time_sync_notification_cb;

    esp_netif_sntp_init(&config);
    esp_netif_sntp_start();

    int retry = 0;
    while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < TIME_REQUEST_RETRY_COUNT) {
        ESP_LOGI(TIME_TAG, "Waiting for system time to be set... (%d/%d)", retry, TIME_REQUEST_RETRY_COUNT);
    }

    esp_netif_sntp_deinit();

    setenv("TZ", TIMEZONE.c_str(), 1);
    tzset();
    timeIsSync = true;
}

void set_wifi_connection_flag(bool isConnected)
{
    wifiIsConnected = isConnected;

    if (wifiIsConnected)
    {
        if (!timeIsSync)
            synctime();
        
        static bool services_started = false;
        if (!services_started) {
            mdns_start();
            http_server_start();
            services_started = true;
        }
    }
}

static void init_gpio_pins()
{
    gpio_reset_pin(HUMIDER_PIN);
    gpio_set_direction(HUMIDER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(HUMIDER_PIN, GPIO_PULLUP_PULLDOWN);

    gpio_reset_pin(LIGHT_PIN);
    gpio_set_direction(LIGHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(LIGHT_PIN, GPIO_PULLUP_PULLDOWN);

    gpio_reset_pin(HEAT_PIN);
    gpio_set_direction(HEAT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(HEAT_PIN, GPIO_PULLUP_PULLDOWN);

    gpio_reset_pin(FOUNTAIN_PIN);
    gpio_set_direction(FOUNTAIN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(FOUNTAIN_PIN, GPIO_PULLUP_PULLDOWN);
}

static void init_bme280_sensor()
{
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_bus = i2c_bus_create(I2C_MASTER_NUM, &conf);
    vTaskDelay(pdMS_TO_TICKS(1000));

    bme280 = bme280_create(i2c_bus, BME280_I2C_ADDRESS_DEFAULT);
    bme280_default_init(bme280);
}

static void init_nvs_flash()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

extern "C" void app_main() 
{
    logs_init();
    original_vprintf = esp_log_set_vprintf(log_vprintf);
    
    init_gpio_pins();
    init_bme280_sensor();
    init_nvs_flash();
    read_settings_nvs();
    
    for(int i = 0; i < 120 ; i++)
    {
        ESP_LOGI("countdown", "%d", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    xTaskCreate(networkTask, "networkTask", 8192, NULL, 10, &networkTaskHandle);
    xTaskCreate(timerTask, "timerTask", 16384, NULL, 10, &timerTaskHandle);
    xTaskCreate(restartTask, "restartTask", 2048, NULL, 10, &restartTaskHandle);
}