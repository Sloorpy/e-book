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

bool BookString::is_ltr_joiner(const uint8_t ch) {
    return ch == '\'' || ch == '@' || ch == '_' || ch == '-' || ch == '.' ||
           ch == '/' || ch == '+' || ch == ':';
}

bool BookString::is_number_separator(const uint8_t ch) {
    return ch == '.' || ch == ',';
}

bool BookString::is_sign_prefix_for_number(const std::vector<uint8_t>& str, const size_t pos) {
    if (pos >= str.size()) {
        return false;
    }

    if (str[pos] != '+' && str[pos] != '-') {
        return false;
    }

    return pos + 1 < str.size() && TextHelper::is_numeric_char(str[pos + 1]);
}

WordType BookString::strong_type_for_byte(const uint8_t ch) {
    if (TextHelper::is_hebrew_char(ch)) {
        return WordType::RTL;
    }

    if (TextHelper::is_english_char(ch)) {
        return WordType::LTR;
    }

    return WordType::NEUTRAL;
}

BookString::BookString(const std::vector<uint8_t>& data)
    : _str(data), _pos(0), _fallback_line_direction(WordType::LTR)
{}

BookString::BookString(const char* str)
    : _str(reinterpret_cast<const uint8_t*>(str), reinterpret_cast<const uint8_t*>(str) + strlen(str)),
      _pos(0),
      _fallback_line_direction(WordType::LTR)
{}

bool BookString::is_char_at(const size_t pos, const char c) const {
    return pos < _str.size() && static_cast<char>(_str[pos]) == c;
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

    const size_t token_end = consume_token_end(_pos);
    const size_t word_end = split_mixed_strong_run_end(_pos, token_end);

    return Word{std::vector<uint8_t>(_str.begin() + _pos, _str.begin() + word_end)};
}

size_t BookString::split_mixed_strong_run_end(const size_t start, const size_t end) const {
    if (start >= end || end > _str.size()) {
        return end;
    }

    WordType active_strong_type = WordType::NEUTRAL;

    for (size_t i = start; i < end; ++i) {
        const WordType byte_type = strong_type_for_byte(_str[i]);

        if (byte_type == WordType::NEUTRAL) {
            continue;
        }

        if (active_strong_type == WordType::NEUTRAL) {
            active_strong_type = byte_type;
            continue;
        }

        if (byte_type != active_strong_type) {
            return i;
        }
    }

    return end;
}

size_t BookString::consume_token_end(const size_t start) const {
    if (start >= _str.size() || is_word_delimiter(_str[start])) {
        return start;
    }

    if (TextHelper::is_hebrew_char(_str[start])) {
        return consume_hebrew_run(start);
    }

    if (TextHelper::is_english_char(_str[start])) {
        return consume_ltr_run(start);
    }

    if (TextHelper::is_numeric_char(_str[start]) || is_sign_prefix_for_number(_str, start)) {
        return consume_number_run(start);
    }

    if (TextHelper::is_sign_char(_str[start])) {
        return consume_neutral_run(start);
    }

    size_t i = start + 1;
    while (i < _str.size() && !is_word_delimiter(_str[i])) {
        ++i;
    }
    return i;
}

size_t BookString::consume_hebrew_run(const size_t start) const {
    size_t i = start;

    while (i < _str.size() && !is_word_delimiter(_str[i])) {
        if (!TextHelper::is_hebrew_char(_str[i])) {
            break;
        }

        ++i;
    }

    return i;
}

size_t BookString::consume_ltr_run(const size_t start) const {
    size_t i = start;

    while (i < _str.size() && !is_word_delimiter(_str[i])) {
        if (TextHelper::is_english_char(_str[i]) || TextHelper::is_numeric_char(_str[i])) {
            ++i;
            continue;
        }

        if (is_ltr_joiner(_str[i]) && i + 1 < _str.size() &&
            (TextHelper::is_english_char(_str[i + 1]) || TextHelper::is_numeric_char(_str[i + 1]))) {
            ++i;
            continue;
        }

        break;
    }

    return i;
}

size_t BookString::consume_number_run(const size_t start) const {
    size_t i = start;

    if (is_sign_prefix_for_number(_str, i)) {
        ++i;
    }

    while (i < _str.size() && !is_word_delimiter(_str[i])) {
        if (TextHelper::is_numeric_char(_str[i])) {
            ++i;
            continue;
        }

        const bool has_prev_digit = i > start && TextHelper::is_numeric_char(_str[i - 1]);
        const bool has_next_digit = i + 1 < _str.size() && TextHelper::is_numeric_char(_str[i + 1]);
        if (is_number_separator(_str[i]) && has_prev_digit && has_next_digit) {
            ++i;
            continue;
        }

        break;
    }

    return i;
}

size_t BookString::consume_neutral_run(const size_t start) const {
    size_t i = start;
    while (i < _str.size() && TextHelper::is_sign_char(_str[i]) && !is_sign_prefix_for_number(_str, i)) {
        ++i;
    }

    return i;
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
    WordType line_direction = WordType::NEUTRAL;

    auto active_line_direction = [&]() {
        if (line_direction == WordType::RTL || line_direction == WordType::LTR) {
            return line_direction;
        }

        return _fallback_line_direction;
    };

    while (!is_end()) {
        Word peek_word = get_word();
        if (peek_word.get().empty()) {
            break;
        }

        if (line_direction == WordType::NEUTRAL &&
            (peek_word.type() == WordType::RTL || peek_word.type() == WordType::LTR)) {
            line_direction = peek_word.type();
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
                if (active_line_direction() == WordType::LTR) {
                    reverse_words.push_back(next_word());
                } else {
                    line.push_back(next_word());
                }
                break;
            case WordType::LTR:
                if (active_line_direction() == WordType::RTL) {
                    reverse_words.push_back(next_word());
                } else {
                    line.push_back(next_word());
                }
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

    if (line_direction == WordType::RTL || line_direction == WordType::LTR) {
        _fallback_line_direction = line_direction;
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
