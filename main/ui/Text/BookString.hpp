#pragma once

#include <cstdint>
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
    static bool is_line_break(uint8_t ch);
    static bool is_word_delimiter(uint8_t ch);
    static bool is_ltr_joiner(uint8_t ch);
    static bool is_number_separator(uint8_t ch);
    static bool is_sign_prefix_for_number(const std::vector<uint8_t>& str, size_t pos);
    static WordType strong_type_for_byte(uint8_t ch);

    size_t consume_token_end(size_t start) const;
    size_t split_mixed_strong_run_end(size_t start, size_t end) const;
    size_t consume_hebrew_run(size_t start) const;
    size_t consume_ltr_run(size_t start) const;
    size_t consume_number_run(size_t start) const;
    size_t consume_neutral_run(size_t start) const;

    void skip_spaces();
    void skip_newline();
    bool is_end() const;
    bool is_space() const;
    bool is_char_at(const size_t pos, const char c) const;

private:
    std::vector<uint8_t> _str;
    size_t _pos;
    WordType _fallback_line_direction;
};
