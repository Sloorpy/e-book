#include "Word.hpp"
#include "TextBox.hpp"
#include "TextHelper.hpp"
#include <algorithm>
#include <vector>

Word::Word(const std::vector<uint8_t>& bytes) :
    _bytes(bytes),
    _type(word_type(bytes))
{
    if (_type == WordType::LTR || _type == WordType::NUMBER) {
        std::reverse(_bytes.begin(), _bytes.end());
    }
}

WordType Word::word_type(const std::vector<uint8_t>& bytes) const
{
    if (bytes.empty()) {
        return WordType::NEUTRAL;
    }

    bool has_rtl = false;
    bool has_ltr = false;
    bool has_number = false;

    for (size_t i = 0; i < bytes.size(); ++i) {
        const uint8_t ch = bytes[i];

        if (TextHelper::is_hebrew_utf8_prefix(ch) && i + 1 < bytes.size()) {
            has_rtl = true;
            ++i;
            continue;
        }

        if (TextHelper::is_hebrew_char(ch)) {
            has_rtl = true;
            continue;
        }

        if (TextHelper::is_english_char(ch)) {
            has_ltr = true;
            continue;
        }

        if (TextHelper::is_numeric_char(ch)) {
            has_number = true;
            continue;
        }
    }

    if (has_rtl && !has_ltr) {
        return WordType::RTL;
    }

    if (has_ltr && !has_rtl) {
        return WordType::LTR;
    }

    if (has_number && !has_ltr && !has_rtl) {
        return WordType::NUMBER;
    }

    return WordType::NEUTRAL;
}

int Word::calc_word_width(const TextBox &tb) const
{
    return calc_word_width(tb.font(), tb.textSize());
}

int Word::calc_word_width(const GFXfont* font, uint8_t textsize) const {
    int width = 0;

    for (size_t i = 0; i < _bytes.size(); ++i) {
        GFXglyph* glyph = TextHelper::get_char_font(_bytes[i], font);
        if (glyph) {
            width += glyph->xAdvance * textsize;
        }
    }
    return width;
}

std::vector<uint8_t> Word::get() const
{
    return _bytes;
}

WordType Word::type() const
{
    return _type;
}
