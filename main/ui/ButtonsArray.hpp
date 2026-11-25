#pragma once

#include "core/HomeButton.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

class ButtonsArray final {
private:
    static constexpr uint16_t button_width = 80;
    static constexpr uint16_t button_radius = 3;
    static constexpr uint16_t button_gap = 3;

public:
    enum class ButtonKind : uint8_t { Settings, Books, Statistics };

    ButtonsArray();
    ButtonKind current() const { return static_cast<ButtonKind>(_current); }
    bool next();
    bool prev();
    void draw(CanvasRegion& row);
    bool select();

private:
    using Buttons = std::array<HomeButton, 3>;
    static Buttons init_buttons();

    Buttons _buttons;
    std::size_t _current = static_cast<std::size_t>(ButtonKind::Books);
    std::optional<size_t> _selected_index;
};
