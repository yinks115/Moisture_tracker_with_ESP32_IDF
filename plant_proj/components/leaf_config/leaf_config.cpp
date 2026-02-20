#include <esp_log.h>
#include "leaf_config.hpp"
#include "leaf_info.hpp"
#include "wifi_credentials.hpp"
#include "esp_mac.h"
#include <cstring>

static const char* _LEAF_CONFIG_LOGGER = "Leaf Congif *** ";

wifi_credentials_t wifi_credentials; //from .hpp file
leaf_info_t leaf_info; //from .hpp file


void initialize_leaf_config() {
    ESP_LOGI(_LEAF_CONFIG_LOGGER, "Initiliazing Leaf Configuration");

    _load_leaf_info();
    _load_wifi_credentials();

    ESP_LOGI(_LEAF_CONFIG_LOGGER, "Initiliazed Leaf Configuration");
}

void _load_leaf_info() {
    ESP_LOGI(_LEAF_CONFIG_LOGGER, "Initiliazing Leaf Info");

    //copies PLANT_NAME value to our object
    strncpy(
        reinterpret_cast<char *>(leaf_info.plant_name),
        PLANT_NAME, sizeof(leaf_info.plant_name) / sizeof(char)
    );

    size_t name_len = strlen(PLANT_NAME);

    //null termination
    leaf_info.plant_name[name_len] = '\0';

    ESP_LOGI(_LEAF_CONFIG_LOGGER, "Initiliazed Leaf Info");
}

void _load_wifi_credentials() {
    ESP_LOGI(_LEAF_CONFIG_LOGGER, "Initiliazing Wifi Credentials");

    //uses strncpy to copy the values stored in WIFI_SSID and WIFI_PASSWORD
    //to the corresponding members of our wifi_credential object

    strncpy(
        reinterpret_cast<char *>(wifi_credentials.wifi_ssid),
        WIFI_SSID, sizeof(wifi_credentials.wifi_ssid) / sizeof(char)
    );

    strncpy(
        reinterpret_cast<char *>(wifi_credentials.wifi_password),
        WIFI_PASSWORD, sizeof(wifi_credentials.wifi_password) / sizeof(char)
    );


    //ensuring that strings are properly null terminated
    size_t ssid_len = strlen(
        (WIFI_SSID)
    );

    size_t pass_len = strlen(
        (WIFI_PASSWORD)
    );

    wifi_credentials.wifi_ssid[ssid_len] = '\0';
    wifi_credentials.wifi_password[pass_len] = '\0';

    ESP_LOGI(_LEAF_CONFIG_LOGGER, "Initiliazed Wifi Credentials");
}