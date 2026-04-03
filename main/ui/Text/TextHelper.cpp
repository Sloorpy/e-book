#include "TextHelper.hpp"
#include "BookString.hpp"

bool TextHelper::is_hebrew_utf8_prefix(uint8_t byte) {
    return byte == TextConstants::HEBREW_UTF8_PREFIX;
}

TextCharKind TextHelper::classify(const std::string& str, size_t index) {
    if (index >= str.size()) {
        return TextCharKind::Text;
    }

    const uint8_t ch = static_cast<uint8_t>(str[index]);

    if (ch == '\n') {
        return TextCharKind::Newline;
    }

    if (is_hebrew_utf8_prefix(ch)) {
        return TextCharKind::Hebrew;
    }

    return TextCharKind::Text;
}

uint8_t TextHelper::get_hebrew_font_char(const std::string& str) {
    if (str.size() < 2) {
        return 0;
    }

    const uint8_t ch = static_cast<uint8_t>(str[0]);
    if (!is_hebrew_utf8_prefix(ch)) {
        return 0;
    }

    const uint16_t utf16 = (static_cast<uint16_t>(ch) << 8) |
                           static_cast<uint8_t>(str[1]);

    return static_cast<uint8_t>(
        utf16 - TextConstants::HEBREW_UTF16_BASE + TextConstants::FONT_OFFSET
    );
}

size_t TextHelper::count_hebrew_chars(const std::string& str) {
    size_t count = 0;
    for (size_t i = 0; i < str.size();) {
        if (is_hebrew_utf8_prefix(static_cast<uint8_t>(str[i]))) {
            ++count;
            i += 2;
        } else {
            ++i;
        }
    }
    return count;
}

GFXglyph* TextHelper::get_char_font(const char letter, const GFXfont* font)
{
    if (font == nullptr) {
        return nullptr;
    }

    const uint8_t ch = static_cast<uint8_t>(letter);
    if (ch < font->first || ch > font->last) {
        return nullptr;
    }

    const uint8_t glyphIndex = ch - font->first;
    return &font->glyph[glyphIndex];
}

bool TextHelper::is_char_in_font_range(uint8_t ch, const GFXfont* font) {
    return font != nullptr && ch >= font->first && ch <= font->last;
}
