#include <freertos/FreeRTOS.h>
#include <esp_event.h>
#include <esp_log.h>
#include "wifi_handler.hpp"
#include <esp_bit_defs.h>

static const char* _WIFI_EVENTS_LOGGER = "Wifi Handler *** ";

esp_err_t wan_event_handler_setup() {
    ESP_LOGI(_WIFI_EVENTS_LOGGER, "Initializing WAN events handler SetUp");
    ESP_LOGI(_WIFI_EVENTS_LOGGER, "Creating wifi event loop");

    bool critical_error = false;
    esp_err_t result = esp_event_loop_create_default(); //creates an event loop. components can publish and subscribe to events in this loop
    if (result != ESP_OK) {
        switch(result){
            case ESP_ERR_NO_MEM:
                ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error! No memory available to create event loop");
                critical_error = true;
                break;
            case ESP_ERR_INVALID_STATE:
                ESP_LOGW(_WIFI_EVENTS_LOGGER, "Warning! Event loop was already created, Continuing ...");
                break;
            case ESP_FAIL:
                ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error! Failed to created event loop");
                critical_error = true;
                break;
            default:
                ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error! Failed to created event loop for unknown reason");
                critical_error = true;
        }
    } else {
        ESP_LOGI(_WIFI_EVENTS_LOGGER, "Wifi Event loop created successfully...Creating Event Group");
    }

    if (critical_error) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Encountered a critical error when setting up event loop");
        return ESP_FAIL;
    }

    //creating an events group
    wifi_events.wan_event_group = xEventGroupCreate(); //returns a pointer to a set of bits. Each bit acts as flag for some type of info
    wifi_events.GOT_IP_BIT = BIT0; //the lsb of events flags(bits) will correspond to whether or not we got an ip address

    if (wifi_events.wan_event_group == NULL) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error! Failed to create Event Group");
        return ESP_FAIL;
    } else {
        ESP_LOGI(_WIFI_EVENTS_LOGGER, "Created Events Group");
    }

    //making subscribers to event (ie event handlers)
    if (esp_event_handler_register(
        WIFI_EVENT,   //the type of event 
        ESP_EVENT_ANY_ID, //the id of the specifc event that falls under the event type. we specified any wifi event
        &wifi_event_handler,
        NULL
    ) == ESP_OK) {
        ESP_LOGI(_WIFI_EVENTS_LOGGER, "Created Wifi event handler");
    } else {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Failed to Create Wifi event handler");
        return ESP_FAIL;
    }

    //ip address is not guranteed to be set once connected to a wifi network so we need to make an event handler
    //specifically for it
    if (esp_event_handler_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &ip_event_handler,
        NULL
    ) == ESP_OK) {
        ESP_LOGI(_WIFI_EVENTS_LOGGER, "Created IP event handler");
    } else {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error! Failed to Create IP event handler");
        return ESP_FAIL;
    }

    return ESP_OK;
}