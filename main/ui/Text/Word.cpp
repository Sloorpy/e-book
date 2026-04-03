#include "Word.hpp"
#include "TextBox.hpp"
#include "TextHelper.hpp"
#include <vector>

int Word::calc_word_width(const TextBox& tb) const {
    return calc_word_width(tb.font(), tb.textSize());
}

int Word::calc_word_width(const GFXfont* font, uint8_t textsize) const {
    int width = 0;

    for (size_t i = 0; i < text.size();) {
        uint8_t byte = static_cast<uint8_t>(text[i]);
        
        if (TextHelper::is_hebrew_utf8_prefix(byte) && i + 1 < text.size()) {
            uint8_t letter = TextHelper::get_hebrew_font_char(text.substr(i, 2));
            if (letter != 0) {
                GFXglyph* glyph = TextHelper::get_char_font(letter, font);
                width += glyph->xAdvance * textsize;
            }
            i += 2;
        } else if (TextHelper::is_char_in_font_range(byte, font)) {
            GFXglyph* glyph = TextHelper::get_char_font(byte, font);
            width += glyph->xAdvance * textsize;
            i++;
        } else {
            i++;
        }
    }
    return width;
}

std::vector<uint8_t> Word::to_font_indices() const {
    std::vector<uint8_t> indices;
    
    for (size_t i = 0; i < text.size();) {
        uint8_t byte = static_cast<uint8_t>(text[i]);
        
        if (TextHelper::is_hebrew_utf8_prefix(byte) && i + 1 < text.size()) {
            uint8_t font_char = TextHelper::get_hebrew_font_char(text.substr(i, 2));
            if (font_char != 0) {
                indices.push_back(font_char);
            }
            i += 2;
        } else {
            indices.push_back(byte);
            i++;
        }
    }
    return indices;
}
