#include "AdcReader.hpp"
#include "Config.hpp"
#include <stdexcept>

AdcReader::AdcReader(const adc_channel_t channel) :
    _adc_handle(),
    _calibration_handle(initialize_calibration()),
    _channel(channel)
{
    configure_adc_channel(channel);
}

AdcReader::~AdcReader()
{
    adc_cali_delete_scheme_line_fitting(_calibration_handle);
}

uint32_t AdcReader::read() const
{
    int output = 0;
    const esp_err_t result = adc_oneshot_get_calibrated_result(_adc_handle.get(), _calibration_handle, _channel, &output);
    if (result != ESP_OK) {
        throw std::runtime_error("Failed to read battery using `adc_oneshot_get_calibrated_result`. Error code: " + std::to_string(result));
    }

    static constexpr uint32_t VOLTAGE_DIDIVOR_MULTIPLIER = 2;
    return static_cast<uint32_t>(output) * VOLTAGE_DIDIVOR_MULTIPLIER;
}

adc_cali_handle_t AdcReader::initialize_calibration()
{
 adc_cali_line_fitting_config_t cali_config = {
        .unit_id = AdcHandle::ADC_UINT,
        .atten = AdcHandle::ATTEN,
        .bitwidth = AdcHandle::BITWIDTH,
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
        .atten = AdcHandle::ATTEN,
        .bitwidth = AdcHandle::BITWIDTH
    };

    const esp_err_t result = adc_oneshot_config_channel(_adc_handle.get(), channel, &channel_config);
    if (result != ESP_OK) {
        throw std::runtime_error("Failed to setup adc CHANNEL for AdcReader reading. Error code: " + std::to_string(result));
    }
}
