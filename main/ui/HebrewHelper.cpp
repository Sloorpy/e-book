#include "HebrewHelper.hpp"
#include "Display.hpp"
#include <cstdint>

bool HebrewHelper::isHebrewUtf8Byte(uint8_t letter) {
    return letter == HebConstants::HEBREW_UTF8_PREFIX;
}

bool HebrewHelper::isAsciiLetter(uint8_t letter) {
    return (letter >= 'A' && letter <= 'Z') || (letter >= 'a' && letter <= 'z');
}

uint8_t HebrewHelper::getHebChar(const char* str, uint32_t i) {
    if (str[i] == '\0' || str[i + 1] == '\0') {
        return 0;
    }

    const uint8_t ch = static_cast<uint8_t>(str[i]);

    if (!isHebrewUtf8Byte(ch)) {
        return 0;
    }

    const uint16_t utf16 = (static_cast<uint16_t>(ch) << 8) |
                           static_cast<uint8_t>(str[i + 1]);

    return static_cast<uint8_t>(
        utf16 - HebConstants::HEBREW_UTF16_BASE + HebConstants::FONT_OFFSET
    );
}

uint32_t HebrewHelper::countEnglishRtl(const char* str, uint32_t start) {
    uint32_t end = start;

    while (str[end] != '\0' && !isHebrewUtf8Byte((uint8_t)str[end])) {
        ++end;
    }

    if (end == start) {
        return 0;
    }

    --end;

    while (end > start && (uint8_t)str[end] == ' ') {
        --end;
    }

    return end - start + 1;
}

std::vector<FontIndex> HebrewHelper::process(const char* str) {
    std::vector<FontIndex> result;
    uint32_t i = 0;
    constexpr uint32_t HEB_CHAR_SIZE = 2;
    constexpr uint8_t INVALID_HEB_CHAR = 0;

    while (str[i] != '\0') {
        const uint8_t ch = (uint8_t)str[i];

        if (isHebrewUtf8Byte(ch)) {
            const uint8_t he_char = getHebChar(str, i);
            if (he_char != INVALID_HEB_CHAR) {
                result.push_back({he_char, false});
            }
            i += HEB_CHAR_SIZE;
            continue;
        }

        if (isAsciiLetter(ch)) {
            uint32_t count = countEnglishRtl(str, i);
            for (uint32_t j = count; j > 0; --j) {
                result.push_back({(uint8_t)str[i + j - 1], true});
            }
            i += count;
            continue;
        }

        result.push_back({ch, false});
        ++i;
    }

    return result;
}

CursorCalculation HebrewHelper::calculateCursor(const Display& display, uint8_t letter, bool is_rtl) {
    CursorCalculation result;
    result.draw_x = display.cursor_x;
    result.draw_y = display.cursor_y;
    result.next_x = display.cursor_x;
    result.next_y = display.cursor_y;

    if (letter == '\r') {
        return result;
    }

    if (letter == '\n') {
        result.next_x = display._width;
        result.next_y = display.cursor_y + static_cast<int16_t>(display.textsize_y) * display.gfxFont->yAdvance;
        return result;
    }

    const uint8_t first = display.gfxFont->first;
    const uint8_t last = display.gfxFont->last;

    if (letter < first || letter > last) {
        return result;
    }

    const GFXglyph* const glyph = display.gfxFont->glyph + letter - first;
    const uint8_t letter_width = glyph->width;
    const uint8_t letter_height = glyph->height;

    if (letter_width == 0 || letter_height == 0) {
        return result;
    }

    const int16_t xo = static_cast<int16_t>(glyph->xOffset);
    const int16_t adv = static_cast<int16_t>(glyph->xAdvance) * static_cast<int16_t>(display.textsize_x);

    int16_t draw_x = is_rtl ? display.cursor_x : display.cursor_x - adv;
    int16_t draw_y = display.cursor_y;

    if (display.wrap && (draw_x + xo * static_cast<int16_t>(display.textsize_x) < 0)) {
        draw_x = display._width;
        draw_y = display.cursor_y + static_cast<int16_t>(display.textsize_y) * display.gfxFont->yAdvance;
    }

    result.draw_x = draw_x;
    result.draw_y = draw_y;
    result.next_x = is_rtl ? draw_x + adv : draw_x;
    result.next_y = draw_y;

    return result;
}
