#pragma once

#include <cstdint>

namespace TextConstants {
    constexpr uint8_t HEBREW_UTF8_PREFIX = 0xD7;
    constexpr uint16_t HEBREW_UTF16_BASE = 0xD790;
    constexpr uint8_t FONT_OFFSET = 0x80;
}

enum class WritingDirection { RTL, LTR };

enum class TextCharKind {
    CarriageReturn,
    Newline,
    Hebrew,
    RtlNeutral,
    LtrRun
};

class TextHelper {
public:
    static bool is_hebrew_utf8_prefix(uint8_t byte);
    static bool is_ascii_letter(uint8_t byte);
    static bool is_ascii_digit(uint8_t byte);
    static bool is_rtl_neutral_ascii(uint8_t byte);
    static TextCharKind classify(const char* str, uint32_t index);
    static bool try_get_hebrew_font_char(const char* str,
                                         uint32_t index,
                                         uint8_t& out_letter);
    static uint32_t count_ltr_run(const char* str, uint32_t start);
};
