#include "mdns_manager.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "mdns.h"

static const char* MDNS_TAG = "mDNS";

void mdns_start() {
    esp_err_t err = mdns_init();
    if (err != ESP_OK) {
        ESP_LOGE(MDNS_TAG, "mDNS init failed: %s", esp_err_to_name(err));
        return;
    }

    mdns_hostname_set("terrarium");
    mdns_instance_name_set("ESP32 Terrarium Control");

    ESP_LOGI(MDNS_TAG, "mDNS hostname set to: terrarium.local");

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"model", "terrarium-control"}
    };

    esp_err_t ret = mdns_service_add("ESP32-Terrarium", "_http", "_tcp", 80, serviceTxtData, sizeof(serviceTxtData) / sizeof(serviceTxtData[0]));
    if (ret != ESP_OK) {
        ESP_LOGE(MDNS_TAG, "mDNS service add failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(MDNS_TAG, "mDNS service added: _http._tcp on port 80");
}

