#pragma once

#include <cstdint>
#include <vector>
#include <gfxfont.h>

class TextBox;

struct Word {
    std::vector<uint8_t> bytes;

    int calc_word_width(const TextBox& tb) const;
    int calc_word_width(const GFXfont* font, uint8_t textsize) const;
};

using Line = std::vector<Word>;
