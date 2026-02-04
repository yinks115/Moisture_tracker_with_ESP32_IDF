#ifndef MOISTURE_SENSOR_HPP
#define MOISTURE_SENSOR_HPP

#include <freertos/FreeRTOS.h>


void moisture_sensor_int();

int get_reading();

esp_err_t _calibrate_adc();

void _populate_buffer();

double _get_robust_mean();


#endif