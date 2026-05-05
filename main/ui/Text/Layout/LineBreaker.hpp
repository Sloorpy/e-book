#pragma once

#include "Text/Layout/TextToken.hpp"

#include <cstddef>
#include <cstdint>
#include <gfxfont.h>

class TextBox;

class LineBreaker final {
public:
    static std::vector<LayoutLine> break_lines(std::vector<ResolvedToken> tokens, const TextBox& text_box);

private:
    LineBreaker(std::vector<ResolvedToken> tokens, const TextBox& text_box);

private:
    std::vector<LayoutLine> run();

private:
    void process_token(ResolvedToken& token);
    bool should_start_new_line(const std::size_t token_width) const;
    void push_current_line();
    std::size_t measure_token_width(const ResolvedToken& token) const;
    std::size_t measure_bytes_width(const std::vector<uint8_t>& bytes) const;

private:
    std::vector<ResolvedToken> _tokens;
    std::vector<LayoutLine> _lines;
    LayoutLine _current_line;
    const GFXfont* _font;
    std::size_t _max_width;
    std::size_t _current_width = 0;
    uint8_t _text_size;
};
