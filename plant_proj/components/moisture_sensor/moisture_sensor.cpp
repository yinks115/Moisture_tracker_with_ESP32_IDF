#include "moisture_sensor.hpp"
#include <esp_log.h>

adc_oneshot_unit_init_cfg_t ADC_CONFIG; 
adc_oneshot_chan_cfg_t CHAN_CONFIG;
adc_cali_line_fitting_config_t CALI_CONFIG;

adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t cali_handle;
std::vector<int> _readings_buffer;
static const char* _logger = "Moisture Sensor *** ";




void moisture_sensor_init(int n) {
    ESP_LOGI(_logger, "Starting Moisture sensor initialization");

    ESP_LOGI(_logger, "Initializing ADC configuration");
    // using ADC1 GPIO(32-39)
    ADC_CONFIG = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&ADC_CONFIG, &adc1_handle));

    ESP_LOGI(_logger, "Initializing ADC channel");
    //channel configuration
    CHAN_CONFIG = {
        .atten = ADC_ATTEN_DB_12, // recommended val for esp32 0V-2.45V
        .bitwidth = ADC_BITWIDTH_DEFAULT, //resolution 12 bits. higher resolution means a more precise measurement
    };

    //selects pin36 (ADC_channel_0) to use for analog readings from the moisture sensor
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &CHAN_CONFIG));

    ESP_LOGI(_logger, "Initializing ADC Calibration");
    //the esp32 compares the input analog reading to a reference voltage. this reference voltage might be off so
    //we need to calibrate it to get a more accurate result
    adc_cali_line_fitting_config_t CALI_CONFIG = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&CALI_CONFIG, &cali_handle));

    ESP_LOGI(_logger, "Succesfully initialized Moisture sensor");

    _readings_buffer.resize(n);
}

double get_moisture_val() {
    ESP_LOGI(_logger, "Getting Moisture value");

    double res;
    _populate_buffer();
    res = _get_mean();

    ESP_LOGI(_logger, "Succesfully Obtained Moisture value of %d", res);
    return res;
}

void _populate_buffer() {
    std::vector<int>::iterator itr;
    int raw_val;
    int cali_val;

    ESP_LOGI(_logger, "Performing %d samples. Populating moisture buffer", _readings_buffer.size());

    for (itr = _readings_buffer.begin(); itr != _readings_buffer.end(); ++itr){

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw_val));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw_val, &cali_val));

        *itr = cali_val;
    }
}

double _get_mean() {
    double sum = 0;
    int len = 0;
    std::vector<int>::iterator itr;

    ESP_LOGI(_logger, "Getting Average of Sampled Readings");

    for (itr = _readings_buffer.begin(); itr != _readings_buffer.end(); ++itr){
        sum += *itr;
        len += 1;
    }

    if (len == 0)
        ESP_LOGW(_logger, "WARNING! called _get_mean on an empty buffer!");

    return sum / len;
}


