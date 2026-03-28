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

class Display;

struct CursorCalculation {
    int16_t draw_x;
    int16_t draw_y;
    int16_t next_x;
    int16_t next_y;
};

class HebrewHelper {
public:
    static bool isHebrewUtf8Byte(uint8_t letter);
    static bool isAsciiLetter(uint8_t letter);
    static uint8_t getHebChar(const char* str, uint32_t i);
    static uint32_t countEnglishRtl(const char* str, uint32_t start);
    static std::vector<FontIndex> process(const char* str);
    static CursorCalculation calculateCursor(const Display& display, uint8_t letter, bool is_rtl);
};
