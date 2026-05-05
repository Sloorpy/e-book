#pragma once
#include "Text/Layout/TextToken.hpp"
#include "Text/Legacy/Word.hpp"

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <gfxfont.h>

namespace TextConstants {
    constexpr uint8_t HEBREW_UTF8_PREFIX = 0xD7;
    constexpr uint8_t FONT_DIFF = 0x10;

    constexpr uint8_t HEBREW_START = 0x80;
    constexpr uint8_t HEBREW_END = 0x9A;

    constexpr uint8_t NUMERIC_START = '0';
    constexpr uint8_t NUMERIC_END = '9';
}

enum class InitialPosition : uint8_t { Left, Center, Right };

class TextHelper {
public:
    static bool is_hebrew_utf8_prefix(uint8_t byte);
    static bool is_hebrew_char(const uint8_t byte);
    static bool is_english_char(const uint8_t byte);
    static bool is_numeric_char(const uint8_t byte);
    static bool is_sign_char(uint8_t byte);
    static bool is_newline_byte(uint8_t byte);

public:
    static size_t count_hebrew_chars(const std::vector<uint8_t>& str);
    static uint16_t line_width(const LegacyLine& line, const TextBox& tb);
    static std::size_t line_width(const LayoutLine& line, const TextBox& tb);
    static std::size_t line_width(const LayoutLine& line, const GFXfont* font, uint8_t text_size);
    static std::size_t token_width(const ResolvedToken& token, const TextBox& tb);
    static std::size_t token_width(const ResolvedToken& token, const GFXfont* font, uint8_t text_size);
    static size_t space_width(const TextBox& tb);
    
public:
    static GFXglyph* get_char_font(const char letter, const GFXfont* font);
    static bool is_char_in_font_range(const uint8_t ch, const GFXfont* font);

public:
    static std::vector<uint8_t> serialize_to_font_indices(const std::vector<uint8_t>& input, const GFXfont* font);
    static std::vector<uint8_t> serialize_to_font_indices(const std::string& input, const GFXfont* font);
};
