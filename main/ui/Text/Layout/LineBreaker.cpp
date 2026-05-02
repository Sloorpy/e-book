#include "Text/Layout/LineBreaker.hpp"

#include "Text/Rendering/TextBox.hpp"

#include <utility>

std::vector<LayoutLine> LineBreaker::break_lines(std::vector<ResolvedToken> tokens, const TextBox& text_box)
{
    (void)text_box;

    std::vector<LayoutLine> lines;
    LayoutLine current_line;
    current_line.reserve(tokens.size());

    // TODO: Replace this placeholder with width-aware wrapping.
    // For now, only explicit newline tokens start a new LayoutLine. The current
    // BookString path still performs the real line wrapping.
    for (ResolvedToken& token : tokens) {
        if (token.token.kind == TokenKind::Newline) {
            lines.push_back(std::move(current_line));
            current_line = LayoutLine{};
            continue;
        }

        current_line.push_back(std::move(token));
    }

    if (!current_line.empty()) {
        lines.push_back(std::move(current_line));
    }

    return lines;
}
