#include <esp_log.h>
#include <esp_sleep.h>
#include <esp_mac.h>
#include "wifi_handler.hpp"
#include "moisture_sensor.hpp"

const uint64_t SLEEP_DURATION = 8000000; //8 million us or 8 s
const int SAMPLE_SIZE = 10;

static const char* _logger = "Moisture Tracker *** "; //scope is restricted to this file via static keyword

void moisture_tracker(void* pvParameters) {

    /*
    Procedure:
    1. get a moisture reading
    2. connect to wifi
    3. send the moisture reading to the backend
    4. disconnect form wifi
    5. go into sleep to optimize power
    */

    ESP_LOGI(_logger, "*************Starting moisture Tracker task****************");
    moisture_sensor_init(SAMPLE_SIZE);
    double reading = get_moisture_val();

    ESP_LOGI(_logger, "Starting wifi connection");
    if (start_wifi_connection() != ESP_OK) {
        ESP_LOGE(_logger, "Encountered an Error when Starting WIFI");
    }

    ESP_LOGI(_logger, "Attempting to POST moisture reading");
    if (post_moisture_reading(reading) != ESP_OK) {
        ESP_LOGE(_logger, "Encountered an Error when POSTING moisture reading");
    }

    ESP_LOGI(_logger, "Disconnecting from wifi");
    if (stop_wifi_connection() != ESP_OK) {
        ESP_LOGE(_logger, "Encountered an Error when stoping WIFI");
    }

    ESP_LOGI(_logger, "Entering Deep Sleep");
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
    esp_deep_sleep_start();

}

