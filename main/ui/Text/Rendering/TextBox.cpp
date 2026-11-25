#include "Text/Rendering/TextBox.hpp"
#include "Display.hpp"
#include "Text/Layout/TextLayout.hpp"
#include <algorithm>
#include <utility>

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

TextBox::TextBox(CanvasRegion& region, int16_t left, int16_t top, int16_t right, int16_t bottom,
                 const GFXfont* font, uint16_t text_size)
    : TextBox(nullptr, left, top, right, bottom, font, text_size)
{
    _region = &region;
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

void TextBox::setTextColor(Color  color) {
    _text_color = static_cast<uint8_t>(color);
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
    const Direction base_direction,
    VerticalAlignment vertical
) {
    if (_font == nullptr) {
        return 0;
    }

    TextPage page = build_page_layout(str, base_direction);
    if (vertical != VerticalAlignment::Top && !page.lines.empty()) {
        const int line_count = static_cast<int>(page.lines.size());
        const int block_height = ink_height() + (line_count - 1) * line_height();
        const int32_t free_height = std::max(0, _bottom - _cursor.y - block_height);
        _cursor.y += vertical == VerticalAlignment::Center ? free_height / 2 : free_height;
    }

    if (!page.lines.empty()) {
        if (_region) {
            draw_page(*_region, page, pos);
        } else if (_display) {
            const uint16_t width = static_cast<uint16_t>(_display->width());
            const uint16_t height = static_cast<uint16_t>(_display->height());
            CanvasRegion screen = _display->create_canvas({0, 0, width, height});
            draw_page(screen, page, pos);
        }
        _cursor.y += page.lines.size() * line_height();
    }

    return page.consumed_bytes;
}

size_t TextBox::write_to_fit(
    const std::vector<uint8_t>& str,
    InitialPosition pos,
    Direction base_direction,
    VerticalAlignment vertical
)
{
    if (!_font || str.empty()) {
        return 0;
    }

    const uint8_t max_size = _textsize;
    while (textSize() > 1 && next_print_size(str, pos, base_direction) < str.size()) {
        setTextSize(textSize() - 1);
    }

    const size_t written = write(str, pos, base_direction, vertical);
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

void TextBox::draw_page(CanvasRegion& target, const TextPage& page, InitialPosition pos) const
{
    const int16_t left = std::max<int16_t>(0, _cursor.x);
    const int16_t top = std::max<int16_t>(0, _top);
    const int16_t right = std::min(_right, target.width());
    const int16_t bottom = std::min(_bottom, target.height());
    if (left >= right || top >= bottom) {
        return;
    }

    CanvasRegion canvas = target.subregion({static_cast<uint16_t>(left), static_cast<uint16_t>(top),
                                            static_cast<uint16_t>(right - left), static_cast<uint16_t>(bottom - top)});
    canvas.setFont(_font);
    int baseline = _cursor.y - top - _glyph_top * _textsize;
    for (const Line& line : page.lines) {
        write_line(canvas, line, pos, _cursor.x - left, baseline);
        baseline += line_height();
    }
}

void TextBox::write_line(CanvasRegion& canvas, const Line& line, InitialPosition pos,
                         int16_t x_offset, int baseline) const
{
    int x = x_offset + line_pen_x(TextHelper::line_width(line, *this), pos);
    for (const ResolvedToken& token : line.tokens) {
        const std::vector<uint8_t>& bytes = token.token.bytes;
        for (std::size_t i = 0; i < bytes.size(); ++i) {
            const std::size_t index = token.direction == Direction::LTR ? bytes.size() - 1 - i : i;
            const uint8_t byte = bytes[index];
            const GFXglyph* glyph = TextHelper::get_char_font(byte, _font);
            if (!glyph) {
                continue;
            }
            x -= glyph->xAdvance * _textsize;
            if (x + (glyph->xOffset + glyph->width) * _textsize > 0 &&
                x + glyph->xOffset * _textsize < canvas.width()) {
                canvas.drawChar(x, baseline, byte, _text_color, static_cast<uint8_t>(Color::WHITE), _textsize);
            }
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
