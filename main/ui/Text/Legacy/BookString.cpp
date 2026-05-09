#include "Text/Legacy/BookString.hpp"
#include "Text/Rendering/TextBox.hpp"
#include "Text/Support/TextHelper.hpp"
#include <vector>
#include <cstring>

BookString::BookString(const std::vector<uint8_t>& data)
    : _str(data), _pos(0) 
{}

BookString::BookString(const char* str)
    : _str(reinterpret_cast<const uint8_t*>(str), reinterpret_cast<const uint8_t*>(str) + strlen(str)), _pos(0) {}

bool BookString::is_char_at(const size_t pos, const char c) const {
    return pos < _str.size() && static_cast<char>(_str[pos]) == c;
}

size_t BookString::find_delimiter(const size_t start) const {
    for (size_t i = start; i < _str.size(); ++i) {
        char c = static_cast<char>(_str[i]);
        if (c == ' ' || c == '\n' || c == '\r') {
            return i;
        }
    }
    return _str.size();
}

void BookString::skip_spaces() {
    while (_pos < _str.size() && static_cast<char>(_str[_pos]) == ' ') {
        ++_pos;
    }
}

void BookString::skip_newline() {
    if (_pos >= _str.size()) {
        return;
    }

    if (is_char_at(_pos, '\r') && is_char_at(_pos + 1, '\n')) {
        _pos += 2;
    } else {
        ++_pos;
    }
}

size_t BookString::next_hebrew_word_size() const {
    if (_pos >= _str.size()) {
        return 0;
    }
    size_t word_end = find_delimiter(_pos);
    return TextHelper::count_hebrew_chars(std::vector<uint8_t>(_str.begin() + _pos, _str.begin() + word_end));
}

bool BookString::is_end() const
{
    return _pos >= _str.size() || 
           is_char_at(_pos, '\n') || 
           is_char_at(_pos, '\r');
}

bool BookString::is_space() const
{
    return _pos < _str.size() && static_cast<char>(_str[_pos]) == ' ';
}

Word BookString::get_word() {
    skip_spaces();

    if (_pos >= _str.size() || is_char_at(_pos, '\n') || is_char_at(_pos, '\r')) {
        return {};
    }

    size_t word_end = find_delimiter(_pos);
    return Word{std::vector<uint8_t>(_str.begin() + _pos, _str.begin() + word_end)};
}

Word BookString::next_word() {
    Word word = get_word();
    if (!word.bytes.empty()) {
        _pos += word.bytes.size();
    }
    return word;
}

LegacyLine BookString::next_line(const TextBox& tb) {
    skip_spaces();

    if (_pos >= _str.size()) {
        return {};
    }

    if (is_char_at(_pos, '\n') || is_char_at(_pos, '\r')) {
        skip_newline();
        return {};
    }

    // TODO: Replace this compatibility path with TextLayout::build_lines().
    // This method currently mixes tokenization, direction handling, line
    // breaking, and byte consumption in one place.
    LegacyLine line;
    LegacyLine reverse_words;
    int line_width = 0;

    while (!is_end()) {
        Word peek_word = get_word();
        if (peek_word.bytes.empty()) {
            break;
        }

        const int word_width = peek_word.calc_word_width(tb);
        const bool has_words = !line.empty() || !reverse_words.empty();
        const int extra_space = has_words ? TextHelper::space_width(tb) : 0;
        const int candidate_width = line_width + extra_space + word_width;

        if (has_words && candidate_width > tb.width()) {
            break;
        }

        if (has_words) {
            line_width += TextHelper::space_width(tb);
        }
        
        switch(peek_word.word_type()) {
            case WordType::HEBREW:
                line.push_back(next_word());
                break;
            case WordType::ENGLISH:
                // TODO: English should become an LTR token/run instead of a
                // reversed word. Signs next to English should be resolved by
                // BidiResolver before rendering.
                reverse_words.push_back(next_word().reverse());
                break;
            case WordType::NUMERIC:
                // TODO: Numbers should keep their internal LTR order while
                // their position is resolved from the surrounding text.
                line.push_back(next_word().reverse());
                break;
            case WordType::SIGN:
                // TODO: Signs should be separate neutral tokens first, then
                // attached to neighboring words/numbers only when rules say so.
                line.push_back(next_word().reverse());
                break;
            default:
                next_word();
                break;
        }
        line_width += word_width;
    }

    for (auto word = reverse_words.rbegin(); word != reverse_words.rend(); ++word) {
        line.emplace_back(*word);
    }

    return line;
}

bool BookString::end() const {
    return _pos >= _str.size();
}

size_t BookString::remaining_bytes() const {
    return _str.size() - _pos;
}
