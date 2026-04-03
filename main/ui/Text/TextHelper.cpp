#include "TextHelper.hpp"
#include <vector>

bool TextHelper::is_hebrew_utf8_prefix(uint8_t byte) {
    return byte == TextConstants::HEBREW_UTF8_PREFIX;
}

TextCharKind TextHelper::classify(const std::vector<uint8_t>& str, size_t index) {
    if (index >= str.size()) {
        return TextCharKind::Text;
    }

    const uint8_t ch = str[index];

    if (ch == '\n') {
        return TextCharKind::Newline;
    }

    if (is_hebrew_utf8_prefix(ch)) {
        return TextCharKind::Hebrew;
    }

    return TextCharKind::Text;
}

uint8_t TextHelper::get_hebrew_font_char(const std::vector<uint8_t>& str) {
    if (str.size() < 2) {
        return 0;
    }

    const uint8_t ch = str[0];
    if (!is_hebrew_utf8_prefix(ch)) {
        return 0;
    }

    const uint16_t utf16 = (static_cast<uint16_t>(ch) << 8) | str[1];

    return static_cast<uint8_t>(
        utf16 - TextConstants::HEBREW_UTF16_BASE + TextConstants::FONT_OFFSET
    );
}

size_t TextHelper::count_hebrew_chars(const std::vector<uint8_t>& str) {
    size_t count = 0;
    for (size_t i = 0; i < str.size();) {
        if (is_hebrew_utf8_prefix(str[i])) {
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

bool TextHelper::is_char_in_font_range(const uint8_t ch, const GFXfont* font) {
    return font != nullptr && ((ch >= font->first && ch <= font->last) || ch == '\n');
}

std::vector<uint8_t> TextHelper::serialize_to_font_indices(
    const std::vector<uint8_t>& input, 
    const GFXfont* font
) {
    std::vector<uint8_t> output;
    output.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        uint8_t ch = input[i];

        if (is_hebrew_utf8_prefix(ch) && i + 1 < input.size()) {
            uint8_t second_byte = input[++i] - TextConstants::FONT_DIFF;
            if (is_char_in_font_range(second_byte, font)) {
                output.emplace_back(second_byte);
            }
        }
        else if (is_char_in_font_range(ch, font)) {
            output.emplace_back(ch);
        }
    }

    output.shrink_to_fit();
    return output;
}

std::vector<uint8_t> TextHelper::serialize_to_font_indices(const std::string &input, const GFXfont *font)
{
    return serialize_to_font_indices(std::vector<uint8_t>(input.begin(),input.end()), font);
}
