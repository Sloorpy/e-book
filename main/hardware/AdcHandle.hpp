#pragma once
#include "esp_adc/adc_oneshot.h"

class AdcHandle final {
public:
    explicit AdcHandle();
    ~AdcHandle();

public:
    adc_oneshot_unit_handle_t get() const;

private:    
    static adc_oneshot_unit_handle_t initialize_adc();

private:
    adc_oneshot_unit_handle_t _handle;

public:
    static constexpr adc_unit_t ADC_UINT = ADC_UNIT_1;
    static constexpr adc_ulp_mode_t NO_ULP = ADC_ULP_MODE_DISABLE;    
    static constexpr adc_atten_t ATTEN = ADC_ATTEN_DB_12;
    static constexpr adc_bitwidth_t BITWIDTH = ADC_BITWIDTH_12;
};
