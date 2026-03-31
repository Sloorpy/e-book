#include "TextBox.hpp"
#include "Display.hpp"
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
    , _direction(dir)
    , _font(nullptr)
    , _textsize_x(1)
    , _textsize_y(1)
    , _textcolor(0)
    , _textbgcolor(0)
    , _wrap(true)
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

void TextBox::setWrap(bool wrap) {
    _wrap = wrap;
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

size_t TextBox::printHebrew(const char* str) {
    if (str == nullptr || _font == nullptr) {
        return 0;
    }

    const GFXfont* const old_font = _display->getFont();
    _display->setFont(_font);

    size_t i = 0;

    while (str[i] != '\0') {
        const uint8_t ch = static_cast<uint8_t>(str[i]);

        if (ch == '\r') {
            ++i;
            continue;
        }

        if (ch == '\n') {
            const int16_t next_y = _cursor_y + line_height();
            if (next_y > _bottom) {
                break;
            }

            _cursor_x = line_start_x();
            _cursor_y = next_y;
            ++i;
            continue;
        }

        // Hebrew UTF-8 character (2 bytes in this project)
        if (HebrewHelper::isHebrewUtf8Byte(ch)) {
            const uint8_t letter = HebrewHelper::getHebChar(str, static_cast<uint32_t>(i));
            if (letter == 0) {
                ++i;
                continue;
            }

            int16_t advance = 0;
            int16_t x_offset = 0;
            int16_t glyph_bottom_offset = 0;
            if (!get_glyph_metrics(letter, advance, x_offset, glyph_bottom_offset)) {
                i += 2;
                continue;
            }

            int16_t draw_x = (_direction == WritingDirection::RTL)
                ? (_cursor_x - advance)
                : _cursor_x;
            int16_t draw_y = _cursor_y;

            if (_wrap) {
                const int16_t glyph_left = draw_x + x_offset * static_cast<int16_t>(_textsize_x);
                const int16_t glyph_right = draw_x + x_offset * static_cast<int16_t>(_textsize_x) + advance;

                bool need_wrap = false;
                if (_direction == WritingDirection::RTL) {
                    need_wrap = (glyph_left < _left);
                } else {
                    need_wrap = (glyph_right > _right);
                }

                if (need_wrap) {
                    draw_y += line_height();
                    if (draw_y > _bottom) {
                        break;
                    }

                    if (_direction == WritingDirection::RTL) {
                        draw_x = _right - advance;
                    } else {
                        draw_x = _left;
                    }
                }
            }

            const int16_t glyph_bottom = draw_y + glyph_bottom_offset;
            if (glyph_bottom > _bottom) {
                break;
            }

            _display->drawChar(
                draw_x,
                draw_y,
                letter,
                _textcolor,
                _textbgcolor,
                _textsize_x,
                _textsize_y
            );

            if (_direction == WritingDirection::RTL) {
                _cursor_x = draw_x;
            } else {
                _cursor_x = draw_x + advance;
            }
            _cursor_y = draw_y;

            i += 2;
            continue;
        }

        // Non-Hebrew run:
        // Draw it as a left-to-right block while consuming original bytes forward,
        // so returned offset stays correct for paging/resume.
        const size_t run_len = HebrewHelper::countEnglishRtl(str, static_cast<uint32_t>(i));
        if (run_len == 0) {
            ++i;
            continue;
        }

        size_t run_pos = 0;
        while (run_pos < run_len) {
            int16_t block_width = 0;
            size_t fit_len = 0;
            bool found_drawable = false;

            // Try to fit as much of the current run as possible on this line.
            while ((run_pos + fit_len) < run_len) {
                const uint8_t run_ch = static_cast<uint8_t>(str[i + run_pos + fit_len]);

                int16_t advance = 0;
                int16_t x_offset = 0;
                int16_t glyph_bottom_offset = 0;

                if (!get_glyph_metrics(run_ch, advance, x_offset, glyph_bottom_offset)) {
                    // Stop the current chunk before unsupported character.
                    if (!found_drawable) {
                        // Consume unsupported character without drawing.
                        ++fit_len;
                    }
                    break;
                }

                found_drawable = true;

                const int16_t next_width = block_width + advance;
                const int16_t block_start_x = _cursor_x - next_width;

                if (_wrap && block_start_x < _left) {
                    break;
                }

                block_width = next_width;
                ++fit_len;
            }

            if (fit_len == 0) {
                const int16_t next_y = _cursor_y + line_height();
                if (next_y > _bottom) {
                    _display->setFont(old_font);
                    return i + run_pos;
                }

                _cursor_x = _right;
                _cursor_y = next_y;
                continue;
            }

            // If we only consumed unsupported bytes, just move forward.
            if (!found_drawable) {
                run_pos += fit_len;
                continue;
            }

            int16_t draw_y = _cursor_y;
            int16_t block_start_x = _cursor_x - block_width;

            if (_wrap && block_start_x < _left) {
                draw_y += line_height();
                if (draw_y > _bottom) {
                    _display->setFont(old_font);
                    return i + run_pos;
                }
                block_start_x = _right - block_width;
            }

            // Check bottom using actual glyph bottoms.
            int16_t max_bottom = draw_y;
            int16_t pen_x = block_start_x;

            for (size_t j = 0; j < fit_len; ++j) {
                const uint8_t run_ch = static_cast<uint8_t>(str[i + run_pos + j]);

                int16_t advance = 0;
                int16_t x_offset = 0;
                int16_t glyph_bottom_offset = 0;
                if (!get_glyph_metrics(run_ch, advance, x_offset, glyph_bottom_offset)) {
                    continue;
                }

                const int16_t glyph_bottom = draw_y + glyph_bottom_offset;
                if (glyph_bottom > max_bottom) {
                    max_bottom = glyph_bottom;
                }

                pen_x += advance;
            }

            if (max_bottom > _bottom) {
                _display->setFont(old_font);
                return i + run_pos;
            }

            // Draw the block left-to-right in original byte order.
            pen_x = block_start_x;
            for (size_t j = 0; j < fit_len; ++j) {
                const uint8_t run_ch = static_cast<uint8_t>(str[i + run_pos + j]);

                int16_t advance = 0;
                int16_t x_offset = 0;
                int16_t glyph_bottom_offset = 0;
                if (!get_glyph_metrics(run_ch, advance, x_offset, glyph_bottom_offset)) {
                    continue;
                }

                _display->drawChar(
                    pen_x,
                    draw_y,
                    run_ch,
                    _textcolor,
                    _textbgcolor,
                    _textsize_x,
                    _textsize_y
                );

                pen_x += advance;
            }

            _cursor_x = block_start_x;
            _cursor_y = draw_y;
            run_pos += fit_len;
        }

        i += run_len;
    }

    _display->setFont(old_font);
    return i;
}