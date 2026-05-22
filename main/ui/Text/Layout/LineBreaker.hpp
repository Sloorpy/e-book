#pragma once

#include "Display.hpp"
#include "Text/Layout/TextToken.hpp"

#include <cstddef>
#include <cstdint>

class TextBox;

class PageSerializer final {
public:
    static std::vector<Line> serialize_lines(
        std::vector<ResolvedToken> tokens,
        const TextBox& text_box,
        Direction base_direction
    );

    static TextPage serialize(
        std::vector<ResolvedToken> tokens,
        const TextBox& text_box,
        Direction base_direction
    );
};
