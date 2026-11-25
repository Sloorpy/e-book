#include "HomeButton.hpp"
#include "Text/Rendering/TextBox.hpp"

#include <algorithm>
#include <utility>
#include <stdexcept>

HomeButton::HomeButton(std::string label, Layout layout)
    : _label(std::move(label)), _layout(layout)
{}

void HomeButton::draw(CanvasRegion& region)
{
    const Rect& bounds = _layout.bounds;
    if (!bounds.fits_within(region.width(), region.height())) {
        throw std::runtime_error("Button bounds must fit within the canvas");
    }

    const int16_t radius = std::clamp<int16_t>(_layout.radius, 0, std::min(bounds.width, bounds.height) / 2);

    Color bg = _selected ? Color::BLACK : Color::WHITE;
    Color text_color = !_selected ? Color::BLACK : Color::WHITE;

    region.fillRoundRect(bounds.x, bounds.y, bounds.width, bounds.height, radius, static_cast<uint16_t>(bg));
    //region.drawRoundRect(bounds.x, bounds.y, bounds.width, bounds.height, radius, static_cast<uint16_t>(Color::BLACK));

    if (_hovered) {
        draw_hover(region);
    }

    TextBox label_box = create_label_box(region);
    label_box.setTextColor(text_color);
    const std::vector<uint8_t> bytes = TextHelper::serialize_to_font_indices(_label, _layout.font);
    label_box.write(bytes, InitialPosition::Center, Direction::LTR, VerticalAlignment::Bottom);
}

void HomeButton::draw_hover(CanvasRegion &region)
{
    const int16_t pos_x = _layout.bounds.x + _layout.bounds.width / 5;
    const int16_t pos_y = _layout.bounds.y + static_cast<int16_t>(_layout.bounds.height) * 0.9;
    const int16_t width = _layout.bounds.width * 3 / 5;
    Color color = _selected ? Color::WHITE : Color::BLACK;

    static constexpr size_t LINE_WIDTH = 4;
    for (size_t i = 0; i < LINE_WIDTH; ++i) {
        region.drawFastHLine(pos_x, pos_y - i, width, static_cast<uint16_t>(color));
    }
}

TextBox HomeButton::create_label_box(CanvasRegion &region) const
{
    constexpr uint16_t side_padding = 2;
    constexpr uint16_t bottom_margin = 8;
    const Rect& bounds = _layout.bounds;
    const uint16_t inset_x = std::min<uint16_t>(side_padding, (bounds.width - 1) / 2);
    const uint16_t inset_bottom = std::min<uint16_t>(bottom_margin, (bounds.height - 1) / 2);
    return TextBox(region, bounds.x + inset_x, bounds.y,
                   bounds.x + bounds.width - inset_x, bounds.y + bounds.height - inset_bottom, _layout.font);
}
