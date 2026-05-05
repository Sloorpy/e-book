#include "Text/Support/TextHelper.hpp"
#include "Text/Rendering/TextBox.hpp"
#include <vector>
#include "TextHelper.hpp"

namespace {

std::size_t bytes_width(const std::vector<uint8_t>& bytes, const GFXfont* font, const uint8_t text_size)
{
    if (font == nullptr) {
        return 0;
    }

    std::size_t width = 0;
    for (const uint8_t byte : bytes) {
        const GFXglyph* const glyph = TextHelper::get_char_font(static_cast<char>(byte), font);
        if (glyph == nullptr) {
            continue;
        }

        width += static_cast<std::size_t>(glyph->xAdvance) *
                 static_cast<std::size_t>(text_size);
    }

    return width;
}

} // namespace

bool TextHelper::is_hebrew_utf8_prefix(uint8_t byte) {
    return byte == TextConstants::HEBREW_UTF8_PREFIX;
}

bool TextHelper::is_hebrew_char(const uint8_t byte)
{
    return is_hebrew_utf8_prefix(byte) ||
           (byte >= TextConstants::HEBREW_START && byte <= TextConstants::HEBREW_END);
}

bool TextHelper::is_english_char(const uint8_t byte)
{
    return (byte >= 'A' && byte <= 'Z') ||
           (byte >= 'a' && byte <= 'z');
}

bool TextHelper::is_numeric_char(const uint8_t byte)
{
    return byte >= TextConstants::NUMERIC_START && byte <= TextConstants::NUMERIC_END;
}

bool TextHelper::is_sign_char(uint8_t byte)
{
    return (byte >= '!' && byte <= '/')  ||
           (byte >= ':' && byte <= '@')  ||
           (byte >= '[' && byte <= '`')  ||
           (byte >= '{' && byte <= '~');
}

bool TextHelper::is_newline_byte(uint8_t byte)
{
    return byte == '\n' || byte == '\r';
}

size_t TextHelper::count_hebrew_chars(const std::vector<uint8_t> &str)
{
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

uint16_t TextHelper::line_width(const Line &line, const TextBox &tb)
{
    if (line.empty()) {
        return 0;
    }

    uint16_t width = 0;
    for (const Word& word: line) {
        width += word.calc_word_width(tb);
        width += TextHelper::space_width(tb);
    }
    width -= TextHelper::space_width(tb);
    return width;
}

std::size_t TextHelper::line_width(const LayoutLine& line, const TextBox& tb)
{
    return line_width(line, tb.font(), tb.textSize());
}

std::size_t TextHelper::line_width(const LayoutLine& line, const GFXfont* font, const uint8_t text_size)
{
    std::size_t width = 0;
    for (const ResolvedToken& token : line) {
        width += token_width(token, font, text_size);
    }

    return width;
}

std::size_t TextHelper::token_width(const ResolvedToken& token, const TextBox& tb)
{
    return token_width(token, tb.font(), tb.textSize());
}

std::size_t TextHelper::token_width(const ResolvedToken& token, const GFXfont* font, const uint8_t text_size)
{
    if (token.token.kind == TokenKind::Newline) {
        return 0;
    }

    return bytes_width(token.token.bytes, font, text_size);
}

size_t TextHelper::space_width(const TextBox &tb)
{
    return static_cast<int16_t>(tb.textSize()) *
           static_cast<int16_t>(tb.font()->glyph[' ' - tb.font()->first].xAdvance);
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
