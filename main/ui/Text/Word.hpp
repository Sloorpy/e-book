#pragma once

#include <string>
#include <vector>
#include <gfxfont.h>

class TextBox;

struct Word {
    std::string text;

    int calc_word_width(const TextBox& tb) const;
    int calc_word_width(const GFXfont* font, uint8_t textsize) const;
    std::vector<uint8_t> to_font_indices() const;
};

using Line = std::vector<Word>;
