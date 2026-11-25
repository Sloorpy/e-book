#pragma once

#include "Display.hpp"
#include "Text/Layout/TextToken.hpp"
#include "Text/Support/TextHelper.hpp"
#include "Text/Legacy/Word.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <gfxfont.h>

enum class VerticalAlignment : uint8_t { Top, Center, Bottom };

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
    TextBox(CanvasRegion& region, int16_t left, int16_t top, int16_t right, int16_t bottom,
            const GFXfont* font, uint16_t text_size = 1);
    TextBox(uint16_t width, uint16_t height, const GFXfont* font, uint16_t text_size = 1);

public:
    // Absolute left and top of the text area. Right and bottom are exclusive.
    void setCursor(Vector2 pos);
    void setCursor(int16_t x, int16_t y);
    void resetCursor();

public:
    void setFont(const GFXfont* font);
    void setTextSize(uint8_t size);
    void setTextColor(Color color);

public:
    size_t write(
        const std::vector<uint8_t>& str,
        InitialPosition pos = InitialPosition::Right,
        Direction base_direction = Direction::RTL,
        VerticalAlignment vertical = VerticalAlignment::Top
    );

    size_t write_to_fit(
        const std::vector<uint8_t>& str,
        InitialPosition pos = InitialPosition::Right,
        Direction base_direction = Direction::RTL,
        VerticalAlignment vertical = VerticalAlignment::Top
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
    int16_t line_start_x() const { return _cursor.x; }

public:
    int line_height() const;
    int ink_height() const;
    int16_t available_width() const;
    int16_t width() const;
    
private:
    TextPage build_page_layout(const std::vector<uint8_t>& str, Direction base_direction) const;
    void draw_page(CanvasRegion& target, const TextPage& page, InitialPosition pos) const;
    void write_line(CanvasRegion& canvas, const Line& line, InitialPosition pos,
                    int16_t x_offset, int baseline) const;
    int line_pen_x(int line_width, InitialPosition pos) const;

    int _glyph_top = 0;
    int _glyph_height = 0;
    int _left_overhang = 0;
    int _right_overhang = 0;

private:
    std::shared_ptr<Display> _display;
    CanvasRegion* _region = nullptr;
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
