#include "Text/Layout/LineBreaker.hpp"

#include "Text/Rendering/TextBox.hpp"
#include "Text/Support/TextHelper.hpp"

#include <utility>

std::vector<LayoutLine> LineBreaker::break_lines(std::vector<ResolvedToken> tokens, const TextBox& text_box)
{
    return LineBreaker(std::move(tokens), text_box).run();
}

LineBreaker::LineBreaker(std::vector<ResolvedToken> tokens, const TextBox& text_box)
    : _tokens(std::move(tokens))
    , _font(text_box.font())
    , _max_width(text_box.width())
    , _text_size(text_box.textSize())
{
    if (!_tokens.empty()) {
        const size_t assumed_size = (_tokens.size() / 8) + 1;
        _lines.reserve(assumed_size);
    }
}

std::vector<LayoutLine> LineBreaker::run()
{
    for (ResolvedToken& token : _tokens) {
        process_token(token);
    }

    if (!_current_line.empty()) {
        _lines.push_back(std::move(_current_line));
    }

    return std::move(_lines);
}

void LineBreaker::process_token(ResolvedToken& token)
{
    if (token.token.kind == TokenKind::Newline) {
        push_current_line();
        return;
    }

    const std::size_t token_width = TextHelper::token_width(token, _font, _text_size);
    if (should_start_new_line(token_width)) {
        push_current_line();
    }

    _current_width += token_width;
    _current_line.push_back(std::move(token));
}

bool LineBreaker::should_start_new_line(const std::size_t token_width) const
{
    return !_current_line.empty() && _max_width > 0 && _current_width + token_width > _max_width;
}

void LineBreaker::push_current_line()
{
    _lines.push_back(std::move(_current_line));
    _current_line = LayoutLine{};
    _current_width = 0;
}
