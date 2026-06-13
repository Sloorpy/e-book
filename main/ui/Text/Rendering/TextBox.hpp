#pragma once

#include "Display.hpp"
#include "Text/Layout/TextToken.hpp"
#include "Text/Support/TextHelper.hpp"
#include "Text/Legacy/Word.hpp"
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
            const GFXfont* font,
            uint16_t text_size = 1
           );

public:
    void setCursor(Vector2 pos);
    void setCursor(int16_t x, int16_t y);
    void resetCursor();

public:
    void setFont(const GFXfont* font);
    void setTextSize(uint8_t size);
    void setTextColor(uint8_t color);

public:
    size_t write(
        const std::vector<uint8_t>& str,
        InitialPosition pos = InitialPosition::Right,
        Direction base_direction = Direction::RTL
    );

    size_t next_print_size(
        const std::vector<uint8_t>& str,
        InitialPosition pos = InitialPosition::Right,
        Direction base_direction = Direction::RTL
    );
    void next_line(InitialPosition pos = InitialPosition::Left, int16_t line_width = 0);

public:
    std::shared_ptr<Display> display() { return _display; }
    const GFXfont* font() const { return _font; }
    uint8_t textSize() const { return _textsize; }
    int16_t left() const { return _left; }
    int16_t bottom() const { return _bottom; }
    int16_t cursor_y() const { return _cursor.y; }
    int16_t line_start_x() const { return _left; }

public:
    int16_t available_width() const;
    int16_t width() const;
    
private:
    TextPage build_page_layout(const std::vector<uint8_t>& str, Direction base_direction) const;
    void write_token(const ResolvedToken& token);
    void write_line(const Line& line , const InitialPosition pos);

    void set_line_cursor(int16_t line_width, InitialPosition pos);
    int16_t glyph_advance(const GFXglyph& glyph) const;
    void draw_char(const Vector2 position, const char letter);
    void write_word(const Word& word);
    void write_legacy_line(const LegacyLine& line, InitialPosition pos);
    int16_t line_height() const;
    bool bottom_reached() const;

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
};
