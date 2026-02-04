#include <esp_log.h>
#include <esp_sleep.h>
#include <esp_mac.h>

const uint64_t SLEEP_DURATION = 8000000; //8 million us or 8 s

static const char* _logger = "Moisture Tracker *** "; //scope is restricted to this file via static keyword

void moisture_tracker(void* pvParameters) {

    /*
    Procedure:
    1. connect to wifi
    2. get a moisture reading
    3. send the moisture reading to the backend
    4. disconnect form wifi
    5. go into sleep to optimize power
    */

    ESP_LOGI(_logger, "Starting moisture Tracker task");

    esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
    esp_deep_sleep_start();

}

