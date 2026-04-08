#include "Word.hpp"
#include "TextBox.hpp"
#include "TextHelper.hpp"
#include <vector>

WordType Word::word_type() const
{
    if (bytes.empty()) {
        return WordType::EMPTY;
    }

    size_t i = 0;
    while (i < bytes.size() && TextHelper::is_sign_char(bytes[i])) {
        ++i;
    }

    if (i >= bytes.size()) {
        return WordType::SIGN;
    }

    if (TextHelper::is_hebrew_char(bytes[i])) {
        return WordType::HEBREW;
    }

    if (TextHelper::is_english_char(bytes[i])) {
        return WordType::ENGLISH;
    }

    if (TextHelper::is_numeric_char(bytes[i])) {
        return WordType::NUMERIC;
    }

    return WordType::EMPTY;
}

int Word::calc_word_width(const TextBox &tb) const
{
    return calc_word_width(tb.font(), tb.textSize());
}

int Word::calc_word_width(const GFXfont* font, uint8_t textsize) const {
    int width = 0;

    for (size_t i = 0; i < bytes.size(); ++i) {
        GFXglyph* glyph = TextHelper::get_char_font(bytes[i], font);
        if (glyph) {
            width += glyph->xAdvance * textsize;
        }
    }
    return width;
}

Word Word::reverse()
{
    std::reverse(bytes.begin(), bytes.end());
    return *this;
}
