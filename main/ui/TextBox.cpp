#include "TextBox.hpp"
#include "Display.hpp"
#include <Fonts/hebEng5x7avia.h>
#include <cstdint>

TextBox::TextBox(std::shared_ptr<Display> display,
                 int16_t left,
                 int16_t top,
                 int16_t right,
                 int16_t bottom,
                 WritingDirection dir)
    : _display(std::move(display))
    , _left(left)
    , _top(top)
    , _right(right)
    , _bottom(bottom)
    , _cursor_x(0)
    , _cursor_y(0)
    , _textsize_x(1)
    , _textsize_y(1)
    , _textcolor(0)
    , _textbgcolor(0)
    , _direction(dir)
    , _font(&hebEng5x7avia)
{
    resetCursor();
}

void TextBox::setCursor(int16_t x, int16_t y) {
    _cursor_x = x;
    _cursor_y = y;
}

void TextBox::resetCursor() {
    _cursor_x = line_start_x();
    _cursor_y = _top;
}

void TextBox::setFont(const GFXfont* font) {
    _font = font;
}

void TextBox::setTextSize(uint8_t size) {
    _textsize_x = size;
    _textsize_y = size;
}

void TextBox::setTextSize(uint8_t sx, uint8_t sy) {
    _textsize_x = sx;
    _textsize_y = sy;
}

void TextBox::setTextColor(uint8_t color) {
    _textcolor = color;
    _textbgcolor = color;
}

void TextBox::setTextColor(uint8_t color, uint8_t bg) {
    _textcolor = color;
    _textbgcolor = bg;
}

int16_t TextBox::line_height() const {
    if (_font == nullptr) {
        return 0;
    }

    return static_cast<int16_t>(_textsize_y) *
           static_cast<int16_t>(_font->yAdvance);
}

int16_t TextBox::line_start_x() const {
    return (_direction == WritingDirection::RTL) ? _right : _left;
}

bool TextBox::get_glyph_metrics(uint8_t letter,
                                int16_t& advance,
                                int16_t& x_offset,
                                int16_t& glyph_bottom_offset) const {
    if (_font == nullptr) {
        return false;
    }

    if (letter < _font->first || letter > _font->last) {
        return false;
    }

    const GFXglyph* const glyph = _font->glyph + (letter - _font->first);

    if (glyph->width == 0 || glyph->height == 0) {
        return false;
    }

    advance = static_cast<int16_t>(glyph->xAdvance) *
              static_cast<int16_t>(_textsize_x);

    x_offset = static_cast<int16_t>(glyph->xOffset);

    glyph_bottom_offset =
        static_cast<int16_t>(glyph->yOffset) * static_cast<int16_t>(_textsize_y) +
        static_cast<int16_t>(glyph->height) * static_cast<int16_t>(_textsize_y) - 1;

    return true;
}

bool TextBox::get_glyph_metrics(uint8_t letter, GlyphMetrics& metrics) const {
    return get_glyph_metrics(
        letter,
        metrics.advance,
        metrics.x_offset,
        metrics.glyph_bottom_offset
    );
}

bool TextBox::move_to_next_line() {
    const int16_t next_y = _cursor_y + line_height();
    if (next_y > _bottom) {
        return false;
    }

    _cursor_x = line_start_x();
    _cursor_y = next_y;
    return true;
}

bool TextBox::handle_newline() {
    return move_to_next_line();
}

bool TextBox::try_place_rtl_flow_glyph(const GlyphMetrics& metrics,
                                       GlyphPlacement& placement) const {
    placement.draw_y = _cursor_y;
    placement.draw_x = (_direction == WritingDirection::RTL)
        ? (_cursor_x - metrics.advance)
        : _cursor_x;

    const int16_t glyph_left =
        placement.draw_x +
        metrics.x_offset * static_cast<int16_t>(_textsize_x);

    const int16_t glyph_right = glyph_left + metrics.advance;

    const bool need_wrap = (_direction == WritingDirection::RTL)
        ? (glyph_left < _left)
        : (glyph_right > _right);

    if (need_wrap) {
        placement.draw_y += line_height();
        if (placement.draw_y > _bottom) {
            return false;
        }

        placement.draw_x = (_direction == WritingDirection::RTL)
            ? (_right - metrics.advance)
            : _left;
    }

    const int16_t glyph_bottom = placement.draw_y + metrics.glyph_bottom_offset;
    if (glyph_bottom > _bottom) {
        return false;
    }

    placement.next_x = (_direction == WritingDirection::RTL)
        ? placement.draw_x
        : static_cast<int16_t>(placement.draw_x + metrics.advance);

    placement.next_y = placement.draw_y;
    return true;
}

void TextBox::draw_glyph(uint8_t letter, const GlyphPlacement& placement) {
    _display->drawChar(
        placement.draw_x,
        placement.draw_y,
        letter,
        _textcolor,
        _textbgcolor,
        _textsize_x,
        _textsize_y
    );

    _cursor_x = placement.next_x;
    _cursor_y = placement.next_y;
}

void TextBox::draw_rtl_flow_glyph(uint8_t letter) {
    GlyphMetrics metrics{};
    if (!get_glyph_metrics(letter, metrics)) {
        return;
    }

    GlyphPlacement placement{};
    if (!try_place_rtl_flow_glyph(metrics, placement)) {
        return;
    }

    draw_glyph(letter, placement);
}

void TextBox::draw_hebrew_glyph(uint8_t letter, const GlyphPlacement& placement) {
    _display->drawChar(
        placement.draw_x,
        placement.draw_y,
        letter,
        _textcolor,
        _textbgcolor,
        _textsize_x,
        _textsize_y
    );

    _cursor_x = placement.next_x;
    _cursor_y = placement.next_y;
}

bool TextBox::handle_hebrew_char(const char* str, size_t& i) {
    uint8_t letter = 0;
    if (!TextHelper::try_get_hebrew_font_char(str, static_cast<uint32_t>(i), letter)) {
        ++i;
        return true;
    }

    draw_rtl_flow_glyph(letter);
    i += 2;
    return true;
}

TextBox::LtrChunk TextBox::measure_ltr_chunk(const char* str,
                                             size_t start,
                                             size_t run_pos,
                                             size_t run_len) const {
    LtrChunk chunk{0, 0, false};

    while ((run_pos + chunk.fit_len) < run_len) {
        const uint8_t ch = static_cast<uint8_t>(str[start + run_pos + chunk.fit_len]);

        GlyphMetrics metrics{};
        if (!get_glyph_metrics(ch, metrics)) {
            if (!chunk.has_drawable) {
                ++chunk.fit_len;
            }
            break;
        }

        chunk.has_drawable = true;

        const int16_t next_width = chunk.block_width + metrics.advance;
        const int16_t block_start_x = _cursor_x - next_width;

        if (block_start_x < _left) {
            break;
        }

        chunk.block_width = next_width;
        ++chunk.fit_len;
    }

    return chunk;
}

int16_t TextBox::compute_ltr_chunk_bottom(const char* str,
                                          size_t start,
                                          size_t run_pos,
                                          size_t fit_len,
                                          int16_t draw_y) const {
    int16_t max_bottom = draw_y;

    for (size_t j = 0; j < fit_len; ++j) {
        const uint8_t ch = static_cast<uint8_t>(str[start + run_pos + j]);

        GlyphMetrics metrics{};
        if (!get_glyph_metrics(ch, metrics)) {
            continue;
        }

        const int16_t glyph_bottom = draw_y + metrics.glyph_bottom_offset;
        if (glyph_bottom > max_bottom) {
            max_bottom = glyph_bottom;
        }
    }

    return max_bottom;
}

void TextBox::draw_ltr_chunk(const char* str,
                             size_t start,
                             size_t run_pos,
                             size_t fit_len,
                             int16_t draw_x,
                             int16_t draw_y) {
    int16_t pen_x = draw_x;

    for (size_t j = 0; j < fit_len; ++j) {
        const uint8_t ch = static_cast<uint8_t>(str[start + run_pos + j]);

        GlyphMetrics metrics{};
        if (!get_glyph_metrics(ch, metrics)) {
            continue;
        }

        _display->drawChar(
            pen_x,
            draw_y,
            ch,
            _textcolor,
            _textbgcolor,
            _textsize_x,
            _textsize_y
        );

        pen_x += metrics.advance;
    }

    _cursor_x = draw_x;
    _cursor_y = draw_y;
}

bool TextBox::handle_ltr_run(const char* str, size_t& i) {
    const size_t run_len = TextHelper::count_ltr_run(str, static_cast<uint32_t>(i));
    if (run_len == 0) {
        ++i;
        return true;
    }

    size_t run_pos = 0;

    while (run_pos < run_len) {
        const LtrChunk chunk = measure_ltr_chunk(str, i, run_pos, run_len);

        if (chunk.fit_len == 0) {
            if (!move_to_next_line()) {
                i += run_pos;
                return false;
            }
            continue;
        }

        if (!chunk.has_drawable) {
            run_pos += chunk.fit_len;
            continue;
        }

        int16_t draw_y = _cursor_y;
        int16_t draw_x = _cursor_x - chunk.block_width;

        if (draw_x < _left) {
            draw_y += line_height();
            if (draw_y > _bottom) {
                i += run_pos;
                return false;
            }

            draw_x = _right - chunk.block_width;
        }

        const int16_t max_bottom =
            compute_ltr_chunk_bottom(str, i, run_pos, chunk.fit_len, draw_y);

        if (max_bottom > _bottom) {
            i += run_pos;
            return false;
        }

        draw_ltr_chunk(str, i, run_pos, chunk.fit_len, draw_x, draw_y);
        run_pos += chunk.fit_len;
    }

    i += run_len;
    return true;
}

bool TextBox::handle_rtl_neutral_ascii(uint8_t ch, size_t& i) {
    draw_rtl_flow_glyph(ch);
    ++i;
    return true;
}

size_t TextBox::printHebrew(const char* str) {
    if (str == nullptr || _font == nullptr) {
        return 0;
    }

    const GFXfont* const old_font = _display->getFont();
    _display->setFont(_font);

    size_t i = 0;

    while (str[i] != '\0') {
        const TextCharKind kind = TextHelper::classify(str, static_cast<uint32_t>(i));

        switch (kind) {
            case TextCharKind::CarriageReturn:
                ++i;
                break;

            case TextCharKind::Newline:
                if (!handle_newline()) {
                    goto done;
                }
                ++i;
                break;

            case TextCharKind::Hebrew:
                if (!handle_hebrew_char(str, i)) {
                    goto done;
                }
                break;

            case TextCharKind::RtlNeutral: {
                const uint8_t ch = static_cast<uint8_t>(str[i]);
                if (!handle_rtl_neutral_ascii(ch, i)) {
                    goto done;
                }
                break;
            }

            case TextCharKind::LtrRun:
                if (!handle_ltr_run(str, i)) {
                    goto done;
                }
                break;
        }
    }

done:
    _display->setFont(old_font);
    return i;
}