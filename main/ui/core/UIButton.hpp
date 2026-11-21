#pragma once

#include "Text/Rendering/TextBox.hpp"

#include <string>

class UIButton final {
public:
    struct Style {
        Vector2 dimensions;
        const GFXfont* font;
        int16_t radius;
    };

    UIButton(std::shared_ptr<Display> display, std::string label);

    void draw_button(Vector2 position, const Style& style,
                     Direction direction = Direction::LTR) const;

private:
    std::shared_ptr<Display> _display;
    std::string _label;
};
