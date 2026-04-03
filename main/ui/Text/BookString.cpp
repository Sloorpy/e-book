#include "BookString.hpp"
#include "TextBox.hpp"
#include "TextHelper.hpp"
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

void BookString::skip_word() {
    Word word = get_word();
    if (!word.bytes.empty()) {
        _pos += word.bytes.size();
    }
    skip_spaces();
}

Word BookString::next_word() {
    Word word = get_word();
    skip_word();
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
    int line_width = 0;

    while (!is_end()) {
        Word peek_word = get_word();
        if (peek_word.bytes.empty()) {
            break;
        }

        int word_width = peek_word.calc_word_width(tb);
        int next_cursor_x = tb.line_start_x() - line_width - word_width;

        if (line_width > 0 && next_cursor_x < tb.left()) {
            break;
        }

        Word word = next_word();
        line.push_back(word);
        line_width += word_width;

        if (!is_end() && !is_space()) {
            int after_space = tb.line_start_x() - line_width - TextHelper::space_width(tb);
            if (after_space < tb.left()) {
                break;
            }
            line_width += TextHelper::space_width(tb);
        }
    }

    return line;
}

bool BookString::end() const {
    return _pos >= _str.size();
}

size_t BookString::remaining_bytes() const {
    return _str.size() - _pos;
}
