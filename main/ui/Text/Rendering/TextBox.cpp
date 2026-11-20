#include "Text/Rendering/TextBox.hpp"
#include "Display.hpp"
#include "Text/Layout/TextLayout.hpp"
#include <algorithm>
#include <utility>

namespace {
// GFX draws custom glyphs relative to a baseline. Limit its pixel writes to
// the text area, including when one unbreakable word is wider than the box.
class TextCanvas final : public Adafruit_GFX {
public:
    TextCanvas(Display& display, int16_t left, int16_t top, int16_t width, int16_t height)
        : Adafruit_GFX(width, height), _display(display), _left(left), _top(top) {}

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (x >= 0 && x < width() && y >= 0 && y < height()) {
            _display.drawPixel(_left + x, _top + y, color);
        }
    }

private:
    Display& _display;
    int16_t _left;
    int16_t _top;
};
}

TextBox::TextBox(std::shared_ptr<Display> display,
                 int16_t left,
                 int16_t top,
                 int16_t right,
                 int16_t bottom,
                 const GFXfont* font,
                 uint16_t text_size) :
    _display(std::move(display)),
    _font(nullptr),
    _left(left),
    _top(top),
    _right(right),
    _bottom(bottom),
    _cursor{0, 0},
    _textsize(text_size),
    _text_color(static_cast<uint8_t>(Color::BLACK))
{
    setFont(font);
    resetCursor();
}

TextBox::TextBox(uint16_t width, uint16_t height, const GFXfont *font, uint16_t text_size) :
    TextBox(nullptr, 0, 0, width, height, font, text_size)
{}

void TextBox::setCursor(Vector2 pos)
{
    setCursor(pos.x, pos.y);
}

void TextBox::setCursor(int16_t x, int16_t y) {
    _cursor = {std::max(x, _left), std::max(y, _top)};
}

void TextBox::resetCursor() {
    _cursor = {_left, _top};
}

void TextBox::setFont(const GFXfont* font) {
    _font = font;
    _glyph_top = 0;
    _glyph_height = 0;
    _left_overhang = 0;
    _right_overhang = 0;
    if (!font) return;

    int bottom = 0;
    for (int ch = font->first; ch <= font->last; ++ch) {
        const GFXglyph& g = font->glyph[ch - font->first];
        if (!g.width || !g.height) continue;
        _glyph_top = std::min(_glyph_top, int(g.yOffset));
        bottom = std::max(bottom, g.yOffset + g.height);
        _left_overhang = std::max(_left_overhang, -g.xOffset);
        _right_overhang = std::max(_right_overhang, g.xOffset + g.width - g.xAdvance);
    }
    _glyph_height = bottom - _glyph_top;
}

void TextBox::setTextSize(uint8_t size) {
    _textsize = size;
}

void TextBox::setTextColor(uint8_t color) {
    _text_color = color;
}

int TextBox::line_height() const
{
    return _font ? _textsize * _font->yAdvance : 0;
}

int TextBox::ink_height() const
{
    return _textsize * _glyph_height;
}

int16_t TextBox::available_width() const
{
    return std::max(0, _right - _cursor.x - (_left_overhang + _right_overhang) * _textsize);
}

int16_t TextBox::width() const
{
    return _right - _left;
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

    TextPage page = build_page_layout(str, base_direction);
    if (_display) {
        TextCanvas canvas(*_display, _cursor.x, _top, _right - _cursor.x, _bottom - _top);
        canvas.setFont(_font);
        for (const Line& line : page.lines) {
            write_line(canvas, line, pos);
            next_line();
        }
    } else {
        _cursor.y += page.lines.size() * line_height();
    }

    return page.consumed_bytes;
}

size_t TextBox::write_to_fit(
    const std::vector<uint8_t>& str,
    InitialPosition pos,
    Direction base_direction
)
{
    if (!_font || str.empty()) {
        return 0;
    }

    const uint8_t max_size = 10;
    while (textSize() > 1 && next_print_size(str, pos, base_direction) < str.size()) {
        setTextSize(textSize() - 1);
    }

    const size_t written = write(str, pos, base_direction);
    setTextSize(max_size);
    return written;
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

    return build_page_layout(str, base_direction).consumed_bytes;
}

TextPage TextBox::build_page_layout(
    const std::vector<uint8_t>& str,
    const Direction base_direction
) const
{
    return TextLayout::build_page(str, base_direction, *this);
}

void TextBox::write_line(Adafruit_GFX& canvas, const Line& line, InitialPosition pos)
{
    int x = line_pen_x(TextHelper::line_width(line, *this), pos);
    const int baseline = _cursor.y - _top - _glyph_top * _textsize;
    auto draw = [&](uint8_t byte) {
        const GFXglyph* glyph = TextHelper::get_char_font(byte, _font);
        if (glyph) {
            x -= glyph->xAdvance * _textsize;
            if (x + (glyph->xOffset + glyph->width) * _textsize > 0 &&
                x + glyph->xOffset * _textsize < canvas.width()) {
                canvas.drawChar(x, baseline, byte, _text_color, static_cast<uint8_t>(Color::WHITE), _textsize);
            }
        }
    };
    for (const ResolvedToken& token : line.tokens) {
        const auto& bytes = token.token.bytes;
        if (token.direction == Direction::LTR) {
            std::for_each(bytes.rbegin(), bytes.rend(), draw);
        } else {
            std::for_each(bytes.begin(), bytes.end(), draw);
        }
    }
}

int TextBox::line_pen_x(int line_width, InitialPosition pos) const
{
    const int left = _left_overhang * _textsize;
    const int spare = available_width() - line_width;
    if (pos == InitialPosition::Left) return left + line_width;
    if (pos == InitialPosition::Center) return left + line_width + (spare + 1) / 2;
    return left + line_width + spare;
}
