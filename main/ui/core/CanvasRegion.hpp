#pragma once

#include "Adafruit_GFX.h"
#include "Geometry.hpp"

class Display;
enum class Color : uint8_t;

class CanvasRegion final : public Adafruit_GFX
{
public:
    ~CanvasRegion() = default;

    CanvasRegion subregion(Rect bounds) const &;
    Rect bounds() const { return _bounds; }
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void fill_screen(Color color) { fillScreen(static_cast<uint16_t>(color)); }

private:
    friend class Display;
    CanvasRegion(Display& display, Rect bounds);
    void setRotation(uint8_t) override {}

    Display& _display;
    const Rect _bounds;

public:
    CanvasRegion(const CanvasRegion&) = delete;
    CanvasRegion& operator=(const CanvasRegion&) = delete;
    CanvasRegion(CanvasRegion&&) = delete;
    CanvasRegion& operator=(CanvasRegion&&) = delete;
};
