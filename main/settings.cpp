#include "settings.h"
#include "esp_log.h"

#include <string.h>
#include <nvs_flash.h>

#include "etl/string.h"

constexpr auto SETTINGS_TAG = "SETTINGS";

// SETTING_TAG

constexpr auto TIMEZONE_TAG = "timezone";

constexpr auto STA_WIFI_SSID_TAG = "sta_wifi_ssid";
constexpr auto STA_WIFI_PASSWORD_TAG = "sta_wifi_pass";

// CURRENT SETTINGS
    etl::string<6> TIMEZONE{};

    etl::string<32> STA_WIFI_SSID{};
    etl::string<64> STA_WIFI_PASSWORD{};

//-----------------------------------

void write_settings_to_nvs() {
    ESP_LOGI(SETTINGS_TAG, "Saving to NVS %s", NVS_NAMESPACE);
    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle));
    // Writing settings
    ESP_ERROR_CHECK(nvs_set_str(my_handle, TIMEZONE_TAG, TIMEZONE.c_str()));

    ESP_ERROR_CHECK(nvs_set_str(my_handle, STA_WIFI_SSID_TAG, STA_WIFI_SSID.c_str()));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, STA_WIFI_PASSWORD_TAG, STA_WIFI_PASSWORD.c_str()));

    // Writing end
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
}

void write_default_settings_to_nvs() {
    ESP_LOGI(SETTINGS_TAG, "Saving to NVS %s", NVS_NAMESPACE);
    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle));
    // Writing settings

    ESP_ERROR_CHECK(nvs_set_str(my_handle, TIMEZONE_TAG, DEFAULT_TIMEZONE));

    ESP_ERROR_CHECK(nvs_set_str(my_handle, STA_WIFI_SSID_TAG, DEFAULT_STA_WIFI_SSID));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, STA_WIFI_PASSWORD_TAG, DEFAULT_STA_WIFI_PASSWORD));

    // Writing end
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
}

void read_str_from_nvs(nvs_handle* my_handle, const char* tag, etl::string<64>& variable) {
    size_t required_size = 0;
    ESP_ERROR_CHECK(nvs_get_str(*my_handle, tag, NULL, &required_size));
    char *buffer = reinterpret_cast<char*>(malloc(required_size));
    ESP_ERROR_CHECK(nvs_get_str(*my_handle, tag, buffer, &required_size));
    variable.assign(buffer);
    free(buffer);
}

void read_str_from_nvs(nvs_handle* my_handle, const char* tag, etl::string<32>& variable) {
    size_t required_size = 0;
    ESP_ERROR_CHECK(nvs_get_str(*my_handle, tag, NULL, &required_size));
    char *buffer = reinterpret_cast<char*>(malloc(required_size));
    ESP_ERROR_CHECK(nvs_get_str(*my_handle, tag, buffer, &required_size));
    variable.assign(buffer);
    free(buffer);
}

void read_str_from_nvs(nvs_handle* my_handle, const char* tag, etl::string<6>& variable) {
    size_t required_size = 0;
    ESP_ERROR_CHECK(nvs_get_str(*my_handle, tag, NULL, &required_size));
    char *buffer = reinterpret_cast<char*>(malloc(required_size));
    ESP_ERROR_CHECK(nvs_get_str(*my_handle, tag, buffer, &required_size));
    variable.assign(buffer);
    free(buffer);
}

void read_settings_nvs() {

    // First start
    nvs_handle my_handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle)  != ESP_OK){
        nvs_close(my_handle);
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
        write_default_settings_to_nvs();
        return;
    }

    //Reading settings

    read_str_from_nvs(&my_handle, TIMEZONE_TAG, TIMEZONE);

    read_str_from_nvs(&my_handle, STA_WIFI_SSID_TAG, STA_WIFI_SSID);
    read_str_from_nvs(&my_handle, STA_WIFI_PASSWORD_TAG, STA_WIFI_PASSWORD);

    ESP_LOGI(SETTINGS_TAG, "TIMEZONE : %s", TIMEZONE.c_str());

    ESP_LOGI(SETTINGS_TAG, "STA_WIFI_SSID : %s", STA_WIFI_SSID.c_str());
    ESP_LOGI(SETTINGS_TAG, "STA_WIFI_PASSWORD : %s", STA_WIFI_PASSWORD.c_str());

    nvs_close(my_handle);
}