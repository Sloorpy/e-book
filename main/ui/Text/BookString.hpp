#pragma once

#include <string>
#include <vector>
#include "Word.hpp"

class TextBox;

class BookString final {
public:
    explicit BookString(const std::string& str);

    Word get_word();
    void skip_word();
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

private:
    std::string _str;
};
