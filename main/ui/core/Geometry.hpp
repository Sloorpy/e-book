#pragma once

#include <stdint.h>

struct Vector2 {
    int16_t x;
    int16_t y;
};

struct Rect {
    uint16_t x, y, width, height;

    constexpr bool fits_within(uint16_t outer_width, uint16_t outer_height) const
    {
        return width > 0 && height > 0 &&
               static_cast<uint32_t>(x) + width <= outer_width &&
               static_cast<uint32_t>(y) + height <= outer_height;
    }
};
