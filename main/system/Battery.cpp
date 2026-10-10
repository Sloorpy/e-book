#include "Battery.hpp"
#include "Config.hpp"
#include <esp_log.h>

Battery &Battery::get()
{
    static Battery instance;
    return instance;
}

Battery::Battery() :
 _adc_reader(Config::Battery::ADC_CHANNEL)
{
}

uint8_t Battery::percentage() const
{
    static constexpr uint16_t NOISE_GROUP_SIZE = 16;
    
    uint32_t total = 0;
    for (uint16_t i = 0; i < NOISE_GROUP_SIZE; ++i) {
        total += calc_battery();
    }
    return static_cast<uint8_t>(total / NOISE_GROUP_SIZE);
}

uint8_t Battery::calc_battery() const
{
    return static_cast<uint8_t>((_adc_reader.read() - Config::Battery::EMPTY_MV) * 100 / (Config::Battery::FULL_MV - Config::Battery::EMPTY_MV));
}
