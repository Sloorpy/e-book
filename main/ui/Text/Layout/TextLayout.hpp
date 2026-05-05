#pragma once

#include "Text/Layout/TextToken.hpp"
#include <cstdint>
#include <vector>

class TextBox;

class TextLayout final {
public:
    // Main entry point for the token-based text pipeline.
    // Expected input is already serialized to the display font indices.
    static std::vector<Line> build_lines(
        const std::vector<uint8_t>& serialized_bytes,
        Direction base_direction,
        const TextBox& text_box
    );
};
