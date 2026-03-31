#pragma once

#include <cstdint>
#include <vector>

struct FontIndex {
    uint8_t index;
    bool is_rtl;
};

namespace HebConstants {
    constexpr uint8_t HEBREW_UTF8_PREFIX = 0xD7;
    constexpr uint16_t HEBREW_UTF16_BASE = 0xD790;
    constexpr uint8_t FONT_OFFSET = 0x80;
    constexpr uint8_t ASCII_SPACE = ' ';
}

enum class WritingDirection { RTL, LTR };

class HebrewHelper {
public:
    static bool isHebrewUtf8Byte(uint8_t letter);
    static bool isAsciiLetter(uint8_t letter);
    static uint8_t getHebChar(const char* str, uint32_t i);

    // Counts a contiguous non-Hebrew run until Hebrew/newline/null.
    // This is now used for block printing while preserving original byte offsets.
    static uint32_t countEnglishRtl(const char* str, uint32_t start);

    // You can keep this if other code still uses it, but TextBox::printHebrew()
    // should no longer rely on it for paging/resume offsets.
    static std::vector<FontIndex> process(const char* str);
};