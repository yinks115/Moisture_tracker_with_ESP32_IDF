#include <freertos/FreeRTOS.h>
#include "esp_mac.h"
#include <esp_mac.h>
#include "nvs_flash.h"
#include <esp_log.h> // logger
#include <esp_heap_caps.h> // used to get the size of the heap (where we allocate memory). this is flash mem on the esp32
#include "moisture_tracker.hpp"

static const char* _main_logger = "MAIN: ";

esp_err_t setup() {
    esp_err_t ret = ESP_OK;
    esp_err_t mem_check = nvs_flash_init();
    if (mem_check != ESP_OK) {
        ESP_LOGE(_main_logger, "Error! Encountered a problem when initializing NVS");
    } else {
        ESP_LOGI(_main_logger, "Successfully initialized NVS");
    }

    size_t mem_size = esp_get_free_heap_size();
    ESP_LOGI(_main_logger, "Memory Avaliable for use: %u bytes", mem_size);

    return ret;
}

extern "C" void app_main(void)
{
    ESP_LOGI("MAIN", "Initializing the Main Function");

    if (setup() != ESP_OK){
        ESP_LOGE(_main_logger, "Encountered an error in setup function");
    }

    xTaskCreate(
        moisture_tracker, // task/function to call
        "moisture_tracker", //the name used to refer to our task
        4096, // allocated 4kb of memory for this task to use
        NULL, // task parameters
        5, //priority. 
        NULL //task handler
    );
    
}