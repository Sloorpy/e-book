#pragma once

#include <cstdint>
#include <vector>
#include <gfxfont.h>

class TextBox;

enum class WordType : uint8_t {
    ENGLISH,
    HEBREW,
    NUMERIC,
    SIGN,
    EMPTY
};

struct Word {
    std::vector<uint8_t> bytes;
    WordType word_type() const;
    int calc_word_width(const TextBox& tb) const;
    int calc_word_width(const GFXfont* font, uint8_t textsize) const;
    Word reverse();
};

using LegacyLine = std::vector<Word>;
