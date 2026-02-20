#ifndef WIFI_HANDLER_HPP
#define WIFI_HANDLER_HPP

#include <freertos/FreeRTOS.h>
#include <esp_event.h> //gives us esp_event_base_t type
#include <freertos/event_groups.h> //gives EventGroupHandle_t type
#include <esp_bit_defs.h>
#include <string>
#include <esp_http_client.h>

typedef struct {
    EventGroupHandle_t wan_event_group;
    int GOT_IP_BIT;
} events_data_t;

extern events_data_t wifi_events;
extern const int HTTP_BUFF_LEN;
extern const char* SERVER_URL;
extern const char* SUBMIT_READING_ROUTE;
extern const int POST_TIMEOUT;
extern const int WIFI_CONNECTION_TIMEOUT;

/**
 * Sends a moisture reading to the backend db
 * 
 * @param reading the moisture reading value to be sent
 * @return returns ESP_OK or ESP_FAIL
 */
esp_err_t post_moisture_reading(const double reading);

/**
 * Set up function for creating wifi events loop and group. in addition this function sets
 * up the wifi and ip event handlers
 * 
 * @return returns an esp_err_t value. if everything went well that value is ESP_OK
 */
esp_err_t wan_event_handler_setup();

/**
 * Registers http event handler and starts the wifi connection
 * 
 * @return returns esp_ok if everything was successful
 */
esp_err_t start_wifi_connection();

/**
 * Stops the wifi connection
 * @return returns ESP_OK or ESP_FAIL
 */
esp_err_t stop_wifi_connection();

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

/**
* Handler for http events
* @param event http event object
* @return esp_err_t returns ESP_OK on success, ESP_FAIL if an error occured, ESP_ERR_INVALID_RESPONSE if event id in event object is invalid
* 
*/
esp_err_t http_event_handler(esp_http_client_event_t* event);

/**
 * returns the mininum of 2 numbers
 * @param n1 first number
 * @param n2 2nd number
 */
int _min(int n1, int n2);

/**
 * Converts the moisture value to a json format
 * 
 * @param value the moisture value
 * @return returns a json string
 */
const std::string _to_json(const double value);

#endif