#include "TextHelper.hpp"

bool TextHelper::is_hebrew_utf8_prefix(uint8_t byte) {
    return byte == TextConstants::HEBREW_UTF8_PREFIX;
}

bool TextHelper::is_ascii_letter(uint8_t byte) {
    return (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z');
}

bool TextHelper::is_ascii_digit(uint8_t byte) {
    return byte >= '0' && byte <= '9';
}

bool TextHelper::is_rtl_neutral_ascii(uint8_t byte) {
    return byte >= ' ' && byte <= '@';
}

TextCharKind TextHelper::classify(const char* str, uint32_t index) {
    if (str == nullptr || str[index] == '\0') {
        return TextCharKind::LtrRun;
    }

    const uint8_t ch = static_cast<uint8_t>(str[index]);

    if (ch == '\r') {
        return TextCharKind::CarriageReturn;
    }

    if (ch == '\n') {
        return TextCharKind::Newline;
    }

    if (is_hebrew_utf8_prefix(ch)) {
        return TextCharKind::Hebrew;
    }

    if (is_rtl_neutral_ascii(ch)) {
        return TextCharKind::RtlNeutral;
    }

    return TextCharKind::LtrRun;
}

bool TextHelper::try_get_hebrew_font_char(const char* str,
                                              uint32_t index,
                                              uint8_t& out_letter) {
    out_letter = 0;

    if (str == nullptr || str[index] == '\0' || str[index + 1] == '\0') {
        return false;
    }

    const uint8_t ch = static_cast<uint8_t>(str[index]);
    if (!is_hebrew_utf8_prefix(ch)) {
        return false;
    }

    const uint16_t utf16 = (static_cast<uint16_t>(ch) << 8) |
                           static_cast<uint8_t>(str[index + 1]);

    out_letter = static_cast<uint8_t>(
        utf16 - TextConstants::HEBREW_UTF16_BASE + TextConstants::FONT_OFFSET
    );

    return true;
}

uint32_t TextHelper::count_ltr_run(const char* str, uint32_t start) {
    uint32_t end = start;

    while (str[end] != '\0') {
        const TextCharKind kind = classify(str, end);

        if (kind == TextCharKind::CarriageReturn ||
            kind == TextCharKind::Newline ||
            kind == TextCharKind::Hebrew ||
            kind == TextCharKind::RtlNeutral) {
            break;
        }

        ++end;
    }

    return end - start;
}
