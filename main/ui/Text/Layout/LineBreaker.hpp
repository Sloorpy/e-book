#pragma once

#include "Display.hpp"
#include "Text/Layout/TextToken.hpp"

#include <cstddef>
#include <cstdint>

class TextBox;

class PageSerializer final {
public:
    static std::vector<Line> serialize_lines(std::vector<ResolvedToken> tokens, const TextBox& text_box);
    static TextPage serialize(std::vector<ResolvedToken> tokens, const TextBox& text_box);

public:
    explicit PageSerializer(const TextBox& text_box);
    bool consume_token(ResolvedToken token);
    TextPage finish();

private:
    bool page_full() const;
    int16_t line_height() const;
    bool should_start_new_line(const std::size_t token_width) const;
    void push_current_line();
    Line finalize_line(Line&& logical_line) const;

private:
    const TextBox& _text_box;
    std::vector<Line> _lines;
    Line _current_line;
    Vector2 _cursor;
    std::size_t _consumed_bytes = 0;
};
