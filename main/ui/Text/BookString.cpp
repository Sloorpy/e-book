#include "BookString.hpp"
#include "TextBox.hpp"
#include "TextHelper.hpp"
#include <vector>
#include <cstring>

bool BookString::is_line_break(const uint8_t ch) {
    return ch == '\n' || ch == '\r';
}

bool BookString::is_word_delimiter(const uint8_t ch) {
    return ch == ' ' || is_line_break(ch);
}

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

    size_t word_end = _pos;
    const uint8_t current = _str[_pos];

    if (TextHelper::is_sign_char(current)) {
        while (word_end < _str.size() && TextHelper::is_sign_char(_str[word_end])) {
            ++word_end;
        }
    }
    else if (TextHelper::is_english_char(current)) {
        while (word_end < _str.size() && TextHelper::is_english_char(_str[word_end])) {
            ++word_end;
        }
    }
    else if (TextHelper::is_numeric_char(current)) {
        while (word_end < _str.size() && TextHelper::is_numeric_char(_str[word_end])) {
            ++word_end;
        }
    }
    else if (TextHelper::is_hebrew_char(current)) {
        while (word_end < _str.size() && !is_word_delimiter(_str[word_end])) {
            if (TextHelper::is_hebrew_utf8_prefix(_str[word_end]) && word_end + 1 < _str.size()) {
                word_end += 2;
                continue;
            }

            if (!TextHelper::is_hebrew_char(_str[word_end])) {
                break;
            }

            ++word_end;
        }
    }
    else {
        word_end = find_delimiter(_pos);
    }

    return Word{std::vector<uint8_t>(_str.begin() + _pos, _str.begin() + word_end)};
}

Word BookString::next_word() {
    Word word = get_word();
    const std::vector<uint8_t> bytes = word.get();

    if (!bytes.empty()) {
        _pos += bytes.size();
    }

    return word;
}

Line BookString::next_line(const TextBox& tb) {
    skip_spaces();

    if (_pos >= _str.size()) {
        return {};
    }

    if (is_char_at(_pos, '\n') || is_char_at(_pos, '\r')) {
        skip_newline();
        return {};
    }

    Line line;
    Line reverse_words;
    int line_width = 0;

    while (!is_end()) {
        Word peek_word = get_word();
        if (peek_word.get().empty()) {
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
        
        switch(peek_word.type()) {
            case WordType::RTL:
                line.push_back(next_word());
                break;
            case WordType::LTR:
                reverse_words.push_back(next_word());
                break;
            case WordType::NUMBER:
                line.push_back(next_word());
                break;
            case WordType::NEUTRAL:
                line.push_back(next_word());
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
