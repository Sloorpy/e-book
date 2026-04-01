#pragma once

#include "HebrewHelper.hpp"
#include <cstdint>
#include <memory>
#include <gfxfont.h>

class Display;

class TextBox {
public:
    TextBox(std::shared_ptr<Display> display,
            int16_t left,
            int16_t top,
            int16_t right,
            int16_t bottom,
            WritingDirection dir = WritingDirection::RTL);

public:
    void setCursor(int16_t x, int16_t y);
    void resetCursor();

public:
    void setFont(const GFXfont* font);
    void setTextSize(uint8_t size);
    void setTextSize(uint8_t sx, uint8_t sy);
    void setTextColor(uint8_t color);
    void setTextColor(uint8_t color, uint8_t bg);
    void setWrap(bool wrap);

public:
    size_t printHebrew(const char* str);

public:
    std::shared_ptr<Display> display() { return _display; }

private:
    int16_t line_height() const;
    int16_t line_start_x() const;

    bool get_glyph_metrics(uint8_t letter,
                           int16_t& advance,
                           int16_t& x_offset,
                           int16_t& glyph_bottom_offset) const;

private:
    std::shared_ptr<Display> _display;
    int16_t _left;
    int16_t _top;
    int16_t _right;
    int16_t _bottom;
    int16_t _cursor_x;
    int16_t _cursor_y;
    uint8_t _textsize_x;
    uint8_t _textsize_y;
    uint8_t _textcolor;
    uint8_t _textbgcolor;
    WritingDirection _direction;
    const GFXfont* _font;
};