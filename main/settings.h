#ifndef SETTINGS_H
#define SETTINGS_H

#define NVS_NAMESPACE "settings"

#include "etl/string.h"

// DEFAULT SETTINGS
    constexpr auto DEFAULT_TIMEZONE = "UTC+13";

    constexpr auto DEFAULT_STA_WIFI_SSID = "Lanios Web";
    constexpr auto DEFAULT_STA_WIFI_PASSWORD = "-Re67Jk22";


// CURRENT SETTINGS
    extern etl::string<6> TIMEZONE;

    extern etl::string<32> STA_WIFI_SSID;
    extern etl::string<64> STA_WIFI_PASSWORD;
    
//-----------------------------------

// FUNCTIONS
void write_settings_to_nvs();
void write_default_settings_to_nvs();
void read_settings_nvs();

#endif //SETTINGS_H