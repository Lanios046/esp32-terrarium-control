#include "wifi.h"
#include "settings.h"
#include "main.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_timer.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include <string.h>

#define DEFAULT_SCAN_LIST_SIZE 32

#define SSID_TIMER_PERIOD_MS 10000
#define PASSWORD_TIMER_PERIOD_MS 10000
#define UNACCESSIBLE_SERVER_TIMER_PERIOD_MS 10000

constexpr auto WIFI_TAG = "Wi-Fi";

static WIFI_STATE currentState = WIFI_STATE::Disconnected;
static uint16_t ap_count = 0;
static uint16_t number = 0;
static wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];

static wifi_scan_config_t scan_config{
	.ssid = NULL,
	.bssid = NULL,
	.channel = 0,
	.show_hidden = true,
	.scan_type = WIFI_SCAN_TYPE_ACTIVE,
	.scan_time = {
		.active = {
			.min = 500,
			.max = 1500
			}
	}
};

static esp_timer_handle_t wifi_timer;

static void retry_timer_callback(void* arg)
{
	switch(currentState)
	{
		case WIFI_STATE::Ready:
		case WIFI_STATE::NoAccessPointOrHidden: {
			esp_wifi_connect();
		} break;
		case WIFI_STATE::NoAccessPoint: {
			wifi_start_scan();
		} break;
		case WIFI_STATE::HasAccessPoint: {
			esp_wifi_connect();
		} break;
		case WIFI_STATE::InvalidPassword: {
			esp_wifi_connect();
		} break;
		case WIFI_STATE::CorrectPassword:
		case WIFI_STATE::ServerIsUnaccessible:
		case WIFI_STATE::Disconnected: break;
	}
}

static void print_auth_mode(int authmode)
{
    switch (authmode) {
    case WIFI_AUTH_OPEN:
        ESP_LOGI(WIFI_TAG, "Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_OWE:
        ESP_LOGI(WIFI_TAG, "Authmode \tWIFI_AUTH_OWE");
        break;
    case WIFI_AUTH_WEP:
        ESP_LOGI(WIFI_TAG, "Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        ESP_LOGI(WIFI_TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        ESP_LOGI(WIFI_TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ESP_LOGI(WIFI_TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_ENTERPRISE:
        ESP_LOGI(WIFI_TAG, "Authmode \tWIFI_AUTH_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        ESP_LOGI(WIFI_TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        ESP_LOGI(WIFI_TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA3_ENT_192:
        ESP_LOGI(WIFI_TAG, "Authmode \tWIFI_AUTH_WPA3_ENT_192");
        break;
    default:
        ESP_LOGI(WIFI_TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}

static void scan_done_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
	ESP_LOGI(WIFI_TAG, "WIFI_EVENT_SCAN_DONE, %d", ap_count);
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));

	for (auto i = 0; i < number; i++) 
	{
        ESP_LOGI(WIFI_TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(WIFI_TAG, "RSSI \t\t%d", ap_info[i].rssi);
        print_auth_mode(ap_info[i].authmode);
        ESP_LOGI(WIFI_TAG, "Channel \t\t%d", ap_info[i].primary);

		if (0 == strcmp((char*)ap_info[i].ssid, STA_WIFI_SSID.c_str()))
		{
			ESP_LOGI(WIFI_TAG, "SSID FOUNDED");
			currentState = WIFI_STATE::HasAccessPoint;
			ESP_ERROR_CHECK( esp_wifi_connect() );
			set_wifi_connection_flag(false);
			return;
		}
    }
	ESP_LOGW(WIFI_TAG, "HAS NO ACCESS POINT WITH CURRENT SSID OR SSID IS HIDDEN");
	currentState = WIFI_STATE::NoAccessPointOrHidden;
	ESP_ERROR_CHECK(esp_timer_start_once(wifi_timer, SSID_TIMER_PERIOD_MS * 1000));
	set_wifi_connection_flag(false);
}

static void disconnect_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		ESP_LOGI(WIFI_TAG, "WIFI_EVENT_STA_DISCONNECTED");
		wifi_event_sta_disconnected_t *status = (wifi_event_sta_disconnected_t *)event_data;
		ESP_LOGW(WIFI_TAG, "REASON %d", status->reason);
    	if (status->reason == WIFI_REASON_AUTH_FAIL 
			|| status->reason == WIFI_REASON_AUTH_EXPIRE 
			|| status->reason == (WIFI_REASON_BEACON_TIMEOUT | WIFI_REASON_ASSOC_TOOMANY))
    	{
			ESP_LOGW(WIFI_TAG, "INVALID PASSWORD");
        	currentState = WIFI_STATE::InvalidPassword;
			ESP_ERROR_CHECK(esp_timer_start_once(wifi_timer, PASSWORD_TIMER_PERIOD_MS * 1000));
			set_wifi_connection_flag(false);
			return;
    	}
		if (status->reason == WIFI_REASON_NO_AP_FOUND )
		{
			ESP_LOGW(WIFI_TAG, "NO ACCESS POINT FOUND");
			currentState = WIFI_STATE::NoAccessPoint;
			ESP_ERROR_CHECK(esp_timer_start_once(wifi_timer, SSID_TIMER_PERIOD_MS * 1000));
			set_wifi_connection_flag(false);
			return;
		} 

		currentState = WIFI_STATE::CorrectPassword;
		ESP_ERROR_CHECK( esp_wifi_connect() );
		set_wifi_connection_flag(false);
	}
}

static void got_ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
		ESP_LOGI(WIFI_TAG, "IP_EVENT_STA_GOT_IP");
		set_wifi_connection_flag(true);
		currentState = WIFI_STATE::Ready;
	}
}

void wifi_initialize() {

	const esp_timer_create_args_t timer_args = {
        .callback = &retry_timer_callback,
        .name = "wifi_timer"
	};

	ESP_ERROR_CHECK(esp_timer_create(&timer_args, &wifi_timer));

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &scan_done_event_handler, NULL) );
	ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_event_handler, NULL) );
	ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &got_ip_event_handler, NULL) );
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

	wifi_config_t sta_config = {};
	if (STA_WIFI_SSID.length() < sizeof(sta_config.sta.ssid)) {
		memcpy(sta_config.sta.ssid, STA_WIFI_SSID.c_str(), STA_WIFI_SSID.length());
	} else {
		ESP_LOGE(WIFI_TAG, "STA SSID too long");
		return;
	}
	
	if (STA_WIFI_PASSWORD.length() < sizeof(sta_config.sta.password)) {
		memcpy(sta_config.sta.password, STA_WIFI_PASSWORD.c_str(), STA_WIFI_PASSWORD.length());
	} else {
		ESP_LOGE(WIFI_TAG, "STA password too long");
		return;
	}

	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK( esp_wifi_set_config(static_cast<wifi_interface_t>(ESP_IF_WIFI_STA), &sta_config) );
	ESP_ERROR_CHECK( esp_wifi_start() );
}

void wifi_start_scan() {
	ap_count = 0;
	number = DEFAULT_SCAN_LIST_SIZE;
    memset(ap_info, 0, sizeof(ap_info));

	ESP_ERROR_CHECK( esp_wifi_scan_start(&scan_config, false) );
}
