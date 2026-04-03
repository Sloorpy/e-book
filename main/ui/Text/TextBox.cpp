#include "TextBox.hpp"
#include "Display.hpp"
#include "BookString.hpp"
#include <Fonts/hebEng5x7avia.h>
#include <cstdint>
#include <cstring>
#include <vector>

TextBox::TextBox(std::shared_ptr<Display> display,
                 int16_t left,
                 int16_t top,
                 int16_t right,
                 int16_t bottom,
                 WritingDirection dir) :
    _display(std::move(display)),
    _font(&hebEng5x7avia),
    _left(left),
    _top(top),
    _right(right),
    _bottom(bottom),
    _cursor{0, 0},
    _textsize(1),
    _text_color(0),
    _direction(dir) 
{
    resetCursor();
}

void TextBox::setCursor(Vector2 pos) {
    _cursor = pos;
}

void TextBox::setCursor(int16_t x, int16_t y) {
    _cursor.x = x;
    _cursor.y = y;
}

void TextBox::resetCursor() {
    _cursor.x = line_start_x();
    _cursor.y = _top;
}

void TextBox::setFont(const GFXfont* font) {
    _font = font;
}

void TextBox::setTextSize(uint8_t size) {
    _textsize = size;
}

void TextBox::setTextColor(uint8_t color) {
    _text_color = color;
}

void TextBox::draw_char(const Vector2 position, const char letter)
{
    _display->drawChar(
                    position.x,
                    position.y,
                    letter,
                    static_cast<uint8_t>(Color::BLACK),
                    static_cast<uint8_t>(Color::WHITE),
                    _textsize
                );
}

int16_t TextBox::line_height() const
{
    if (_font == nullptr) {
        return 0;
    }
    return static_cast<int16_t>(_textsize) *
            static_cast<int16_t>(_font->yAdvance);
}

int16_t TextBox::available_width() const {
    return _cursor.x - _left;
}

int16_t TextBox::space_width() const {
    return static_cast<int16_t>(_textsize) *
           static_cast<int16_t>(_font->glyph[' ' - _font->first].xAdvance);
}

size_t TextBox::print_hebrew(const char* str) {
    if (str == nullptr || _font == nullptr) {
        return 0;
    }

    _display->setFont(_font);
    BookString bs(str);
    const size_t original_len = strlen(str);

    while (!bs.end()) {
        const Line line = bs.next_line(*this);

        if (line.empty()) {
            if (_cursor.y > _bottom) {
                break;
            }
            continue;
        }

        write_line(line);
        next_line();

        if (_cursor.y > _bottom) {
            break;
        }
    }

    return original_len - bs.remaining_bytes();
}

void TextBox::write_word(const Word& word)
{
    const std::vector<uint8_t> font_indices = word.to_font_indices();

    int16_t pen_x = _cursor.x;

    for (uint8_t ch : font_indices) {
        const GFXglyph* const glyph = TextHelper::get_char_font(ch, _font);
        if (glyph == nullptr) {
            continue;
        }

        pen_x -= static_cast<int16_t>(glyph->xAdvance) *
                    static_cast<int16_t>(_textsize);
        
        draw_char(Vector2{pen_x, _cursor.y}, ch);
    }

    _cursor.x = pen_x;
}

void TextBox::write_line(const Line &line)
{
    const Word& last_word = line.back();
    for (const Word& word : line) {
        write_word(word);

        if (&word != &last_word) {
            _cursor.x -= space_width();

            if (_cursor.x < left()) {
                break;
            }
        }
    }
}

void TextBox::next_line()
{
    _cursor.y += line_height();
    _cursor.x = line_start_x();
}

size_t TextBox::next_print_size(const char *str)
{
    if (str == nullptr || _font == nullptr) {
        return 0;
    }
    const size_t original_len = strlen(str);
    const Vector2 original_cursor = _cursor;

    BookString bs(str);

    while (!bs.end()) {
        const Line line = bs.next_line(*this);

        if (line.empty()) {
            if (_cursor.y > _bottom) {
                break;
            }
            continue;
        }

        const Word& last_word = line.back();
        for (const Word& word : line) {
            const int16_t word_width = static_cast<int16_t>(word.calc_word_width(_font, _textsize));
            _cursor.x -= word_width;

            if (&word != &last_word) {
                _cursor.x -= space_width();

                if (_cursor.x < _left) {
                    break;
                }
            }
        }

        next_line();

        if (_cursor.y > _bottom) {
            break;
        }
    }

    _cursor = original_cursor;
    return original_len - bs.remaining_bytes();
}
