#include "CanvasRegion.hpp"
#include "Display.hpp"

#include <stdexcept>

CanvasRegion::CanvasRegion(Display& display, Rect bounds)
    : Adafruit_GFX(bounds.width, bounds.height),
      _display(display), _bounds(bounds)
{}

CanvasRegion CanvasRegion::subregion(Rect bounds) const &
{
    if (!bounds.fits_within(_bounds.width, _bounds.height)) {
        throw std::runtime_error("Canvas bounds must fit within the parent region");
    }
    bounds.x += _bounds.x;
    bounds.y += _bounds.y;
    return _display.create_canvas(bounds);
}

void CanvasRegion::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if (x < 0 || x >= width() || y < 0 || y >= height()) {
        return;
    }

    _display.drawPixel(_bounds.x + x, _bounds.y + y, color);
}
