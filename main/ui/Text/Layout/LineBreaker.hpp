#pragma once

#include "Text/Layout/TextToken.hpp"

class TextBox;

class LineBreaker final {
public:
    // Converts resolved tokens into physical screen lines.
    // Once implemented, this should be the only place that knows about wrapping.
    static LayoutLines break_lines(ResolvedTokens tokens, const TextBox& text_box);

private:
    // TODO: Add measuring helpers here.
    // Line breaking should measure ResolvedToken::token.bytes with TextBox font
    // metrics and preserve enough token information to report consumed bytes.
};
