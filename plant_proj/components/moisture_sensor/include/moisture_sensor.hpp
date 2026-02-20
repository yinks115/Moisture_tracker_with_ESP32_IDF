#ifndef MOISTURE_SENSOR_HPP
#define MOISTURE_SENSOR_HPP

#include <freertos/FreeRTOS.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <vector>

/**
 * Espressif Docs on using one shot mode and calibrating analog readings
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc_oneshot.html
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc_calibration.html
 * 
 * data sheet
 * https://documentation.espressif.com/esp32_technical_reference_manual_en.pdf
 */

extern adc_oneshot_unit_init_cfg_t ADC_CONFIG; // adc object used to select ADC unit
extern adc_oneshot_chan_cfg_t CHAN_CONFIG; //used to set attenuation and bitwidth
extern adc_cali_line_fitting_config_t CALI_CONFIG; // used for calibration


/**
 * Sets up the ADC on the ESP32 as well as a buffer of size n to store moisture readings
 * @param n size of the buffer used to capture buff_size reading
 */
void moisture_sensor_init(int n);

/**
 * performs n reads from the moisture sensor and stores them in a buffer.
 * multisampling is done to mitigate noise
 * @return returns the average of the buffer values
 */
double get_moisture_val();

/**
 * reads from the configured gpio pin and populates the buffer
 */
void _populate_buffer();

/**
 * gets the mean of the buffer
 */
double _get_mean();


#endif