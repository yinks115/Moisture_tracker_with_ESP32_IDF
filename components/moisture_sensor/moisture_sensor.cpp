#include "moisture_sensor.hpp"

adc_oneshot_unit_init_cfg_t ADC_CONFIG; 
adc_oneshot_chan_cfg_t CHAN_CONFIG;
adc_cali_line_fitting_config_t CALI_CONFIG;

adc_oneshot_unit_handle_t adc1_handle;
std::vector<double> _readings_buffer;



void moisture_sensor_int(int n) {
    // using ADC1 GPIO(32-39)
    ADC_CONFIG = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&ADC_CONFIG, &adc1_handle));

    //channel configuration
    CHAN_CONFIG = {
        .atten = ADC_ATTEN_DB_12, // recommended val for esp32 0V-2.45V
        .bitwidth = ADC_BITWIDTH_DEFAULT, //resolution 12 bits. higher resolution means a more precise measurement
    };

    //selects pin36 (ADC_channel_0) to use for analog readings from the moisture sensor
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &CHAN_CONFIG));

    //the esp32 compares the input analog reading to a reference voltage. this reference voltage might be off so
    //we need to calibrate it to get a more accurate result
    adc_cali_line_fitting_config_t CALI_CONFIG = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    adc_cali_create_scheme_line_fitting(&CALI_CONFIG, &)

}
