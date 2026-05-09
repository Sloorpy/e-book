#include "Text/Layout/LineBreaker.hpp"

#include "Text/Rendering/TextBox.hpp"
#include "Text/Support/TextHelper.hpp"

#include <utility>

std::vector<Line> PageSerializer::serialize_lines(std::vector<ResolvedToken> tokens, const TextBox& text_box)
{
    return serialize(std::move(tokens), text_box).lines;
}

TextPage PageSerializer::serialize(std::vector<ResolvedToken> tokens, const TextBox& text_box)
{
    PageSerializer serializer(text_box);
    for (ResolvedToken& token : tokens) {
        serializer.consume_token(std::move(token));
    }

    return serializer.finish();
}

PageSerializer::PageSerializer(const TextBox& text_box)
    : _text_box(text_box)
    , _cursor{0, text_box.cursor_y()}
{}

bool PageSerializer::consume_token(ResolvedToken token)
{
    if (page_full()) {
        return false;
    }

    const std::size_t token_bytes = token.token.bytes.size();

    if (token.token.kind == TokenKind::Newline) {
        _current_line.consumed_bytes += token_bytes;
        _consumed_bytes += token_bytes;
        push_current_line();
        return true;
    }

    const std::size_t token_width = TextHelper::token_width(token, _text_box);
    if (should_start_new_line(token_width)) {
        push_current_line();
        if (page_full()) {
            return false;
        }
    }

    _current_line.consumed_bytes += token_bytes;
    _consumed_bytes += token_bytes;

    if (_current_line.empty() && token.token.kind == TokenKind::Space) {
        return true;
    }

    _cursor.x += static_cast<int16_t>(token_width);
    _current_line.tokens.push_back(std::move(token));
    return true;
}

TextPage PageSerializer::finish()
{
    if (!_current_line.empty() || _current_line.consumed_bytes > 0) {
        push_current_line();
    }

    return TextPage{std::move(_lines), _consumed_bytes};
}

bool PageSerializer::page_full() const
{
    if (line_height() <= 0) {
        return true;
    }

    return _cursor.y > _text_box.bottom();
}

int16_t PageSerializer::line_height() const
{
    const GFXfont* font = _text_box.font();
    if (font == nullptr) {
        return 0;
    }

    return static_cast<int16_t>(_text_box.textSize()) *
           static_cast<int16_t>(font->yAdvance);
}

bool PageSerializer::should_start_new_line(const std::size_t token_width) const
{
    return !_current_line.empty() &&
           _text_box.width() > 0 &&
           static_cast<std::size_t>(_cursor.x) + token_width > static_cast<std::size_t>(_text_box.width());
}

void PageSerializer::push_current_line()
{
    _lines.push_back(std::move(_current_line));
    _cursor.y += line_height();

    _current_line = Line{};
    _cursor.x = 0;
}
