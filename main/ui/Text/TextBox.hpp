#pragma once

#include "Display.hpp"
#include "Text/TextHelper.hpp"
#include "Text/Word.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <gfxfont.h>

class TextBox {
public:
    TextBox(std::shared_ptr<Display> display,
            int16_t left,
            int16_t top,
            int16_t right,
            int16_t bottom,
            WritingDirection dir = WritingDirection::RTL);

public:
    void setCursor(Vector2 pos);
    void setCursor(int16_t x, int16_t y);
    void resetCursor();

public:
    void setFont(const GFXfont* font);
    void setTextSize(uint8_t size);
    void setTextColor(uint8_t color);

public:
    size_t print_hebrew(const std::vector<uint8_t>& str);
    size_t next_print_size(const std::vector<uint8_t>& str);
    void write_word(const Word& word);
    void write_line(const Line& line);
    void next_line();

public:
    std::shared_ptr<Display> display() { return _display; }
    const GFXfont* font() const { return _font; }
    uint8_t textSize() const { return _textsize; }
    int16_t left() const { return _left; }
    int16_t line_start_x() const { return (_direction == WritingDirection::RTL) ? _right - 1 : _left; }

public:
    int16_t available_width() const;
    int16_t space_width() const;

private:
        void draw_char(const Vector2 position, const char letter);

    int16_t line_height() const;

private:
    std::shared_ptr<Display> _display;
    const GFXfont* _font;

private:
    int16_t _left;
    int16_t _top;
    int16_t _right;
    int16_t _bottom;

private:
    Vector2 _cursor;
    uint8_t _textsize;
    uint8_t _text_color;
    WritingDirection _direction;
};
