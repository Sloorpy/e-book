#pragma once

#include <cstdint>
#include <vector>
#include <gfxfont.h>

class TextBox;

enum class WordType : uint8_t {
    RTL,
    LTR,
    NUMBER,
    NEUTRAL
};

class Word final {
public:
    explicit Word(const std::vector<uint8_t>& bytes = {});
    ~Word() = default;

public:
    int calc_word_width(const TextBox& tb) const;
    int calc_word_width(const GFXfont* font, uint8_t textsize) const;
    std::vector<uint8_t> get() const;
    WordType type() const;

private:
    WordType word_type(const std::vector<uint8_t>& bytes) const;

private:
    std::vector<uint8_t> _bytes;
    WordType _type;
};

using Line = std::vector<Word>;
