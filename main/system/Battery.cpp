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
    return static_cast<uint8_t>((_adc_reader.read() - Config::Battery::EMPTY_MV) * 100 / (Config::Battery::FULL_MV - Config::Battery::EMPTY_MV));
}
