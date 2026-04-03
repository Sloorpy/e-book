#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <gfxfont.h>

namespace TextConstants {
    constexpr uint8_t HEBREW_UTF8_PREFIX = 0xD7;
    constexpr uint16_t HEBREW_UTF16_BASE = 0xD790;
    constexpr uint8_t FONT_OFFSET = 0x80;
    constexpr uint8_t FONT_DIFF = 0x10;
}

enum class WritingDirection { RTL, LTR };

enum class TextCharKind {
    Newline,
    Hebrew,
    Text
};

class TextHelper {
public:
    static bool is_hebrew_utf8_prefix(uint8_t byte);
    static TextCharKind classify(const std::vector<uint8_t>& str, size_t index);
    static uint8_t get_hebrew_font_char(const std::vector<uint8_t>& str);
    static size_t count_hebrew_chars(const std::vector<uint8_t>& str);
    static GFXglyph* get_char_font(const char letter, const GFXfont* font);
    static bool is_char_in_font_range(uint8_t ch, const GFXfont* font);
    static std::vector<uint8_t> serialize_to_font_indices(const std::vector<uint8_t>& input, const GFXfont* font);
    static std::vector<uint8_t> serialize_to_font_indices(const std::string& input, const GFXfont* font);
};
