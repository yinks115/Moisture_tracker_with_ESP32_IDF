#ifndef WIFI_HANDLER_HPP
#define WIFI_HANDLER_HPP

#include <freertos/FreeRTOS.h>
#include <esp_event.h> //gives us esp_event_base_t type
#include <freertos/event_groups.h> //gives EventGroupHandle_t type
#include <esp_bit_defs.h>

typedef struct {
    EventGroupHandle_t wan_event_group;
    int GOT_IP_BIT;
} events_data_t;

extern events_data_t wifi_events;

/** 
* Handler for Wifi events
* 
* @param arg pointer to the event source
* @param event_base event base associated with the event
* @param event_id Id of the event
* @param event_data data associated with the event
*/
void wifi_event_handler(
    void* arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data //will be NULL
);

/** 
* Handler for IP events
* 
* @param arg pointer to the event source
* @param event_base event base associated with the event
* @param event_id Id of the event
* @param event_data data associated with the event
*/
void ip_event_handler(
    void* arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data //will be NULL
);

#endif