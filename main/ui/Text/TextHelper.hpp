#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <gfxfont.h>

namespace TextConstants {
    constexpr uint8_t HEBREW_UTF8_PREFIX = 0xD7;
    constexpr uint16_t HEBREW_UTF16_BASE = 0xD790;
    constexpr uint8_t FONT_OFFSET = 0x80;
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
    static TextCharKind classify(const std::string& str, size_t index);
    static uint8_t get_hebrew_font_char(const std::string& str);
    static size_t count_hebrew_chars(const std::string& str);
    static GFXglyph* get_char_font(const char letter, const GFXfont* font);
    static bool is_char_in_font_range(uint8_t ch, const GFXfont* font);
};
