#include "Word.hpp"
#include "TextBox.hpp"
#include "TextHelper.hpp"
#include <vector>

int Word::calc_word_width(const TextBox& tb) const {
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
