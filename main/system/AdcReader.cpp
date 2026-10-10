#include "AdcReader.hpp"
#include "Config.hpp"
#include <stdexcept>

static constexpr adc_unit_t ADC_UINT = ADC_UNIT_1;
static constexpr adc_ulp_mode_t NO_ULP = ADC_ULP_MODE_DISABLE;
static constexpr adc_atten_t ATTEN = ADC_ATTEN_DB_12;
static constexpr adc_bitwidth_t BITWIDTH = ADC_BITWIDTH_12;

AdcReader::AdcReader(const adc_channel_t channel) :
    _adc_handle(initialize_adc()),
    _calibration_handle(initialize_calibration()),
    _channel(channel)
{
    configure_adc_channel(channel);
}

AdcReader::~AdcReader()
{
    try {
        adc_cali_delete_scheme_line_fitting(_calibration_handle);
        adc_oneshot_del_unit(_adc_handle);
    }
    catch (...) {

    }
}

uint32_t AdcReader::read() const
{
    int output = 0;
    const esp_err_t result = adc_oneshot_get_calibrated_result(_adc_handle, _calibration_handle, _channel, &output);
    if (result != ESP_OK) {
        throw std::runtime_error("Failed to read battery using `adc_oneshot_get_calibrated_result`. Error code: " + std::to_string(result));
    }
    return static_cast<uint32_t>(output);
}

adc_oneshot_unit_handle_t AdcReader::initialize_adc()
{
    adc_oneshot_unit_handle_t handle;
    adc_oneshot_unit_init_cfg_t adc_config {
        .unit_id = ADC_UINT,
        .ulp_mode = NO_ULP
    };    
    
    esp_err_t result = adc_oneshot_new_unit(&adc_config, &handle);
    if (result != ESP_OK) {
        throw std::runtime_error("Failed to setup adc HANDLE for AdcReader reading. Error code: " + std::to_string(result));
    }

    return handle;
}

adc_cali_handle_t AdcReader::initialize_calibration()
{
 adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UINT,
        .atten = ATTEN,
        .bitwidth = BITWIDTH,
    };
    
    adc_cali_handle_t handle;
    const esp_err_t result = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
    if (result != ESP_OK) {
        throw std::runtime_error("Failed to setup adc CALIBRATION for AdcReader reading. Error code: " + std::to_string(result));
    }
    return handle;
}

void AdcReader::configure_adc_channel(const adc_channel_t channel)
{
    adc_oneshot_chan_cfg_t channel_config {
        .atten = ATTEN,
        .bitwidth = BITWIDTH
    };

    const esp_err_t result = adc_oneshot_config_channel(_adc_handle, channel, &channel_config);
    if (result != ESP_OK) {
        throw std::runtime_error("Failed to setup adc CHANNEL for AdcReader reading. Error code: " + std::to_string(result));
    }
}
