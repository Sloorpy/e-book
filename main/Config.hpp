#pragma once
#include <cstdint>
#include <cstddef>
#include "esp_adc/adc_cali_scheme.h"

namespace Config {
    namespace Display {
        constexpr uint16_t PAGE_WIDTH = 300;
        constexpr uint16_t PAGE_HEIGHT = 400;
        constexpr uint16_t BITMAP_WIDTH = 160;
        constexpr uint16_t BITMAP_HEIGHT = 210;
        constexpr size_t INITIAL_PAGE_WINDOW_SIZE = 2048;
    }
   

    namespace Battery {
        static constexpr adc_channel_t ADC_CHANNEL = ADC_CHANNEL_6;
        static constexpr uint32_t FULL_MV = 4200; 
        static constexpr uint32_t EMPTY_MV = 3100; 
    }
}