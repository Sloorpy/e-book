#include "BookString.hpp"
#include "TextBox.hpp"
#include "TextHelper.hpp"

BookString::BookString(const std::string& str)
    : _str(str) {}

void BookString::skip_spaces() {
    while (!_str.empty() && _str[0] == ' ') {
        _str = _str.substr(1);
    }
}

void BookString::skip_newline() {
    if (_str.empty()) {
        return;
    }

    if (_str[0] == '\r' && _str.length() > 1 && _str[1] == '\n') {
        _str = _str.substr(2);
    } else {
        _str = _str.substr(1);
    }
}

size_t BookString::next_hebrew_word_size() const {
    if (_str.empty()) {
        return 0;
    }
    size_t word_end = _str.find_first_of(" \n\r");
    if (word_end == std::string::npos) {
        word_end = _str.size();
    }
    std::string word_str = _str.substr(0, word_end);
    return TextHelper::count_hebrew_chars(word_str);
}

bool BookString::is_end() const
{
    return _str.empty() || _str[0] == '\n' || _str[0] == '\r';
}

bool BookString::is_space() const
{
    return !_str.empty() && _str[0] == ' ';
}

Word BookString::get_word() {
    skip_spaces();

    if (_str.empty() || _str[0] == '\n' || _str[0] == '\r') {
        return {""};
    }

    size_t word_end = _str.find_first_of(" \n\r");
    if (word_end == std::string::npos) {
        word_end = _str.size();
    }
    std::string word_str = _str.substr(0, word_end);
    return {word_str};
}

void BookString::skip_word() {
    Word word = get_word();
    if (!word.text.empty()) {
        _str = _str.substr(word.text.size());
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

    if (_str.empty()) {
        return {};
    }

    if (_str[0] == '\n' || _str[0] == '\r') {
        skip_newline();
        return {};
    }

    Line line;
    int line_width = 0;

    while (!is_end()) {
        Word peek_word = get_word();
        if (peek_word.text.empty()) {
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
            int after_space = tb.line_start_x() - line_width - tb.space_width();
            if (after_space < tb.left()) {
                break;
            }
            line_width += tb.space_width();
        }
    }

    return line;
}

bool BookString::end() const {
    return _str.empty();
}

size_t BookString::remaining_bytes() const {
    return _str.size();
}
