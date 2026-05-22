#include "Text/Rendering/TextBox.hpp"
#include "Display.hpp"
#include "Text/Layout/TextLayout.hpp"
#include "ScopedDisplayFont.hpp"
#include <Fonts/hebEng5x7avia.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

TextBox::TextBox(std::shared_ptr<Display> display,
                 int16_t left,
                 int16_t top,
                 int16_t right,
                 int16_t bottom,
                 const GFXfont* font,
                 uint16_t text_size) :
    _display(std::move(display)),
    _font(font),
    _left(left),
    _top(top),
    _right(right),
    _bottom(bottom),
    _cursor{0, 0},
    _textsize(text_size),
    _text_color(static_cast<uint8_t>(Color::BLACK))
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

bool TextBox::bottom_reached() const
{
    return _cursor.y > _bottom;
}

int16_t TextBox::available_width() const {
    return _cursor.x - _left;
}

int16_t TextBox::width() const
{
    return _right - _left;
}


void TextBox::write_word(const Word& word)
{
    int16_t pen_x = _cursor.x;

    for (uint8_t ch : word.bytes) {
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

void TextBox::write_legacy_line(const LegacyLine &line, InitialPosition pos)
{
    if (line.empty()) {
        return;
    }

    const int16_t line_width = static_cast<int16_t>(TextHelper::line_width(line, *this));
    set_line_cursor(line_width, pos);

    const Word& last_word = line.back();
    for (const Word& word : line) {
        write_word(word);

        if (&word != &last_word) {
            _cursor.x -= TextHelper::space_width(*this);

            if (_cursor.x < left()) {
                break;
            }
        }
    }
}

void TextBox::next_line(InitialPosition pos, int16_t line_width)
{
    (void)pos;
    (void)line_width;

    _cursor.y += line_height();
}

size_t TextBox::write(
    const std::vector<uint8_t>& str,
    const InitialPosition pos,
    const Direction base_direction
) {
    if (_font == nullptr) {
        return 0;
    }

    ScopedDisplayFont display_font(*_display, _font);

    TextPage page = build_page_layout(str, base_direction);

    for (const Line& line: page.lines) {
        write_line(line, pos);
    }

    return page.consumed_bytes;
}

size_t TextBox::next_print_size(
    const std::vector<uint8_t>& str,
    const InitialPosition pos,
    const Direction base_direction
)
{
    (void)pos;

    if (_font == nullptr) {
        return 0;
    }

    ScopedDisplayFont display_font(*_display, _font);

    return build_page_layout(str, base_direction).consumed_bytes;
}

TextPage TextBox::build_page_layout(
    const std::vector<uint8_t>& str,
    const Direction base_direction
) const
{
    return TextLayout::build_page(str, base_direction, *this);
}

void TextBox::write_token(const ResolvedToken &token)
{
    std::vector<uint8_t> bytes = token.token.bytes;

    if (token.direction == Direction::LTR) {
        std::reverse(bytes.begin(), bytes.end());
    }

    for (const uint8_t byte: bytes) {
        const GFXglyph* const glyph = TextHelper::get_char_font(byte, _font);
        if (glyph == nullptr) {
            continue;
        }

        _cursor.x -= glyph_advance(*glyph);
        draw_char(Vector2{_cursor.x, _cursor.y}, byte);
    }
}

void TextBox::write_line(const Line &line, const InitialPosition pos)
{
    const int16_t line_width = static_cast<int16_t>(TextHelper::line_width(line, *this));
    set_line_cursor(line_width, pos);

    for (const ResolvedToken& token: line.tokens) {
        write_token(token);
    }

    next_line(pos, line_width);
}

void TextBox::set_line_cursor(const int16_t line_width, const InitialPosition pos)
{
    switch (pos) {
        case InitialPosition::Left:
            _cursor.x = _left + line_width;
            break;
        case InitialPosition::Center:
            _cursor.x = _right - (width() - line_width) / 2;
            break;
        case InitialPosition::Right:
            _cursor.x = _right;
            break;
    }
}

int16_t TextBox::glyph_advance(const GFXglyph& glyph) const
{
    return static_cast<int16_t>(glyph.xAdvance) * static_cast<int16_t>(_textsize);
}
