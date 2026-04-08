#pragma once

#include <vector>
#include "Word.hpp"

class TextBox;

class BookString final {
public:
    explicit BookString(const std::vector<uint8_t>& data);
    explicit BookString(const char* str);

    Word get_word();
    Word next_word();
    Line next_line(const TextBox& tb);
    bool end() const;
    size_t remaining_bytes() const;

private:
    void skip_spaces();
    void skip_newline();
    size_t next_hebrew_word_size() const;
    bool is_end() const;
    bool is_space() const;
    bool is_char_at(const size_t pos, const char c) const;
    size_t find_delimiter(const size_t start) const;

private:
    std::vector<uint8_t> _str;
    size_t _pos;
};
