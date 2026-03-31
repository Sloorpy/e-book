#include "HebrewHelper.hpp"
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

    while (str[end] != '\0' &&
           str[end] != '\n' &&
           str[end] != '\r' &&
           !isHebrewUtf8Byte(static_cast<uint8_t>(str[end]))) {
        ++end;
    }

    return end - start;
}

std::vector<FontIndex> HebrewHelper::process(const char* str) {
    std::vector<FontIndex> result;
    uint32_t i = 0;
    constexpr uint32_t HEB_CHAR_SIZE = 2;
    constexpr uint8_t INVALID_HEB_CHAR = 0;

    while (str[i] != '\0') {
        const uint8_t ch = static_cast<uint8_t>(str[i]);

        if (isHebrewUtf8Byte(ch)) {
            const uint8_t he_char = getHebChar(str, i);
            if (he_char != INVALID_HEB_CHAR) {
                result.push_back({he_char, false});
            }
            i += HEB_CHAR_SIZE;
            continue;
        }

        if (isAsciiLetter(ch)) {
            const uint32_t count = countEnglishRtl(str, i);
            for (uint32_t j = count; j > 0; --j) {
                result.push_back({static_cast<uint8_t>(str[i + j - 1]), true});
            }
            i += count;
            continue;
        }

        result.push_back({ch, false});
        ++i;
    }

    return result;
}