#pragma once

#include "TextHelper.hpp"
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
    struct GlyphMetrics {
        int16_t advance;
        int16_t x_offset;
        int16_t glyph_bottom_offset;
    };

    struct GlyphPlacement {
        int16_t draw_x;
        int16_t draw_y;
        int16_t next_x;
        int16_t next_y;
    };

    struct LtrChunk {
        size_t fit_len;
        int16_t block_width;
        bool has_drawable;
    };

private:
    int16_t line_height() const;
    int16_t line_start_x() const;

    bool get_glyph_metrics(uint8_t letter,
                           int16_t& advance,
                           int16_t& x_offset,
                           int16_t& glyph_bottom_offset) const;

    bool get_glyph_metrics(uint8_t letter, GlyphMetrics& metrics) const;

    bool move_to_next_line();

    bool handle_newline();
    bool handle_hebrew_char(const char* str, size_t& i);
    bool handle_ltr_run(const char* str, size_t& i);

    bool try_place_rtl_flow_glyph(const GlyphMetrics& metrics,
                                  GlyphPlacement& placement) const;

    void draw_glyph(uint8_t letter, const GlyphPlacement& placement);
    void draw_rtl_flow_glyph(uint8_t letter);

    void draw_hebrew_glyph(uint8_t letter, const GlyphPlacement& placement);

    LtrChunk measure_ltr_chunk(const char* str,
                               size_t start,
                               size_t run_pos,
                               size_t run_len) const;

    int16_t compute_ltr_chunk_bottom(const char* str,
                                     size_t start,
                                     size_t run_pos,
                                     size_t fit_len,
                                     int16_t draw_y) const;

    void draw_ltr_chunk(const char* str,
                        size_t start,
                        size_t run_pos,
                        size_t fit_len,
                        int16_t draw_x,
                        int16_t draw_y);
    bool handle_rtl_neutral_ascii(uint8_t ch, size_t& i);

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