#pragma once
#include "AdcHandle.hpp"
class AdcReader final {
public:
    explicit AdcReader(const adc_channel_t channel);
    ~AdcReader();

public:
    uint32_t read() const;

private:
    static adc_oneshot_unit_handle_t initialize_adc();
    static adc_cali_handle_t initialize_calibration();
    void configure_adc_channel(const adc_channel_t channel);

private:
    AdcHandle _adc_handle;
    adc_cali_handle_t _calibration_handle;
    adc_channel_t _channel;
};