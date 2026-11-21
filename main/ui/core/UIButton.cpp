#include "UIButton.hpp"

#include <algorithm>
#include <utility>

UIButton::UIButton(std::shared_ptr<Display> display, std::string label)
    : _display(std::move(display)), _label(std::move(label))
{}

void UIButton::draw_button(Vector2 position, const Style& style,
                           Direction direction) const
{
    const auto dimensions = style.dimensions;

    if (dimensions.x <= 0 || dimensions.y <= 0) {
        return;
    }

    const auto radius = std::clamp<int16_t>(style.radius, 0, std::min(dimensions.x, dimensions.y) / 2);
    const auto white = static_cast<uint16_t>(Color::WHITE);
    const auto black = static_cast<uint16_t>(Color::BLACK);
    _display->fillRoundRect(position.x, position.y, dimensions.x, dimensions.y, radius, white);
    _display->drawRoundRect(position.x, position.y, dimensions.x, dimensions.y, radius, black);

    // Inset the text area so it stays inside the border and rounded corners.
    const int16_t inset = std::max<int16_t>(1, radius);
    TextBox label_box(_display,
                      position.x + inset, position.y + inset,
                      position.x + dimensions.x - inset, position.y + dimensions.y - inset,
                      style.font);
    const auto bytes = TextHelper::serialize_to_font_indices(_label, style.font);
    label_box.write(bytes, InitialPosition::Center, direction, VerticalAlignment::Center);
}
