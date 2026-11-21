#include "AdcHandle.hpp"
#include <stdexcept>

AdcHandle::AdcHandle() :
    _handle(initialize_adc())
{
}

AdcHandle::~AdcHandle()
{
    adc_oneshot_del_unit(_handle);
}

adc_oneshot_unit_handle_t AdcHandle::get() const
{
    return _handle;
}

adc_oneshot_unit_handle_t AdcHandle::initialize_adc()
{
    adc_oneshot_unit_handle_t handle;
    adc_oneshot_unit_init_cfg_t adc_config {
        .unit_id = AdcHandle::ADC_UINT,
        .ulp_mode = AdcHandle::NO_ULP
    };    
    
    esp_err_t result = adc_oneshot_new_unit(&adc_config, &handle);
    if (result != ESP_OK) {
        throw std::runtime_error("Failed to setup adc HANDLE for AdcReader reading. Error code: " + std::to_string(result));
    }

    return handle;
}