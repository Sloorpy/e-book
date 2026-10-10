#include "Text/Layout/LineBreaker.hpp"
#include "Text/Rendering/TextBox.hpp"
#include "Text/Support/TextHelper.hpp"

#include <algorithm>
#include <utility>
#include <vector>

namespace {

struct WordGroup final {
    std::vector<ResolvedToken> tokens;
    Direction context_direction;
    std::size_t consumed_bytes = 0;

    bool is_newline() const
    {
        return tokens.size() == 1 && tokens.front().token.kind == TokenKind::Newline;
    }

    bool is_space() const
    {
        return tokens.size() == 1 && tokens.front().token.kind == TokenKind::Space;
    }
};

bool is_break_group(const WordGroup& group)
{
    return group.is_space() || group.is_newline();
}

bool is_hebrew_word(const ResolvedToken& token)
{
    return token.token.kind == TokenKind::HebrewWord;
}

bool is_english_word(const ResolvedToken& token)
{
    return token.token.kind == TokenKind::EnglishWord;
}

bool is_word_or_number(const ResolvedToken& token)
{
    return token.token.kind == TokenKind::HebrewWord ||
           token.token.kind == TokenKind::EnglishWord ||
           token.token.kind == TokenKind::Number;
}

bool is_sign_char(const ResolvedToken& token, const uint8_t ch)
{
    return token.token.kind == TokenKind::Sign &&
           token.token.bytes.size() == 1 &&
           token.token.bytes.front() == ch;
}

bool is_leading_sign(const ResolvedToken& token)
{
    return is_sign_char(token, '(') ||
           is_sign_char(token, '[') ||
           is_sign_char(token, '{') ||
           is_sign_char(token, '$');
}

bool is_trailing_sign(const ResolvedToken& token)
{
    return is_sign_char(token, '.') ||
           is_sign_char(token, ',') ||
           is_sign_char(token, ':') ||
           is_sign_char(token, ';') ||
           is_sign_char(token, '!') ||
           is_sign_char(token, '?') ||
           is_sign_char(token, '%') ||
           is_sign_char(token, ')') ||
           is_sign_char(token, ']') ||
           is_sign_char(token, '}');
}

bool is_connector_sign(const ResolvedToken& token)
{
    return is_sign_char(token, '-');
}

bool can_connect_with_dash(const ResolvedToken& before, const ResolvedToken& after)
{
    return is_word_or_number(before) && is_word_or_number(after);
}

std::size_t token_consumed_bytes(const ResolvedToken& token)
{
    return token.token.bytes.size();
}

std::size_t group_width(const WordGroup& group, const TextBox& text_box)
{
    std::size_t width = 0;
    for (const ResolvedToken& token : group.tokens) {
        width += TextHelper::token_width(token, text_box);
    }

    return width;
}

bool contains_hebrew_word(const WordGroup& group)
{
    return std::any_of(group.tokens.begin(), group.tokens.end(), is_hebrew_word);
}

bool contains_english_word(const WordGroup& group)
{
    return std::any_of(group.tokens.begin(), group.tokens.end(), is_english_word);
}

Direction resolve_group_context(const WordGroup& group, const Direction last_word_direction)
{
    if (contains_hebrew_word(group)) {
        return Direction::RTL;
    }

    if (contains_english_word(group)) {
        return Direction::LTR;
    }

    return last_word_direction;
}

void update_last_word_direction(const WordGroup& group, Direction& last_word_direction)
{
    if (contains_hebrew_word(group)) {
        last_word_direction = Direction::RTL;
    } else if (contains_english_word(group)) {
        last_word_direction = Direction::LTR;
    }
}

WordGroup make_group(std::vector<ResolvedToken> tokens, Direction& last_word_direction)
{
    WordGroup group{std::move(tokens), last_word_direction, 0};

    for (const ResolvedToken& token : group.tokens) {
        group.consumed_bytes += token_consumed_bytes(token);
    }

    group.context_direction = resolve_group_context(group, last_word_direction);
    update_last_word_direction(group, last_word_direction);
    return group;
}

void append_pending_leading_signs(
    std::vector<ResolvedToken>& group_tokens,
    std::vector<ResolvedToken>& pending_leading_signs
)
{
    group_tokens.reserve(group_tokens.size() + pending_leading_signs.size());
    for (ResolvedToken& sign : pending_leading_signs) {
        group_tokens.push_back(std::move(sign));
    }

    pending_leading_signs.clear();
}

void normalize_space_contexts(std::vector<WordGroup>& groups, const Direction base_direction)
{
    for (std::size_t i = 0; i < groups.size(); ++i) {
        if (!groups[i].is_space()) {
            continue;
        }

        const Direction previous_context = [&]() {
            for (std::size_t j = i; j > 0; --j) {
                const WordGroup& previous = groups[j - 1];
                if (!is_break_group(previous)) {
                    return previous.context_direction;
                }
                if (previous.is_newline()) {
                    break;
                }
            }
            return base_direction;
        }();

        const Direction next_context = [&]() {
            for (std::size_t j = i + 1; j < groups.size(); ++j) {
                const WordGroup& next = groups[j];
                if (!is_break_group(next)) {
                    return next.context_direction;
                }
                if (next.is_newline()) {
                    break;
                }
            }
            return base_direction;
        }();

        groups[i].context_direction = previous_context == next_context
            ? previous_context
            : base_direction;
    }
}

std::vector<WordGroup> build_word_groups(std::vector<ResolvedToken> tokens, const Direction base_direction)
{
    std::vector<WordGroup> groups;
    std::vector<ResolvedToken> pending_leading_signs;
    Direction last_word_direction = base_direction;

    for (std::size_t i = 0; i < tokens.size();) {
        if (tokens[i].token.kind == TokenKind::Newline || tokens[i].token.kind == TokenKind::Space) {
            if (!pending_leading_signs.empty()) {
                groups.push_back(make_group(std::move(pending_leading_signs), last_word_direction));
                pending_leading_signs = {};
            }

            std::vector<ResolvedToken> group_tokens;
            group_tokens.push_back(std::move(tokens[i++]));
            groups.push_back(make_group(std::move(group_tokens), last_word_direction));
            continue;
        }

        if (is_leading_sign(tokens[i]) && i + 1 < tokens.size()) {
            const TokenKind next_kind = tokens[i + 1].token.kind;
            if (next_kind != TokenKind::Space && next_kind != TokenKind::Newline) {
                pending_leading_signs.push_back(std::move(tokens[i++]));
                continue;
            }
        }

        std::vector<ResolvedToken> group_tokens;
        append_pending_leading_signs(group_tokens, pending_leading_signs);
        group_tokens.push_back(std::move(tokens[i++]));

        bool extended = true;
        while (extended && i < tokens.size()) {
            extended = false;

            if (i + 1 < tokens.size() &&
                is_connector_sign(tokens[i]) &&
                !group_tokens.empty() &&
                can_connect_with_dash(group_tokens.back(), tokens[i + 1])) {
                group_tokens.push_back(std::move(tokens[i++]));
                group_tokens.push_back(std::move(tokens[i++]));
                extended = true;
                continue;
            }

            if (is_trailing_sign(tokens[i])) {
                group_tokens.push_back(std::move(tokens[i++]));
                extended = true;
            }
        }

        groups.push_back(make_group(std::move(group_tokens), last_word_direction));
    }

    if (!pending_leading_signs.empty()) {
        groups.push_back(make_group(std::move(pending_leading_signs), last_word_direction));
    }

    normalize_space_contexts(groups, base_direction);

    return groups;
}

class PageSerializerRunner final {
public:
    PageSerializerRunner(const TextBox& text_box, const Direction base_direction)
        : _text_box(text_box)
        , _base_direction(base_direction)
        , _cursor{0, text_box.cursor_y()}
    {}

    TextPage serialize(std::vector<WordGroup> groups)
    {
        for (WordGroup& group : groups) {
            if (!consume_group(std::move(group))) {
                break;
            }
        }

        return finish();
    }

private:
    bool consume_group(WordGroup group)
    {
        if (page_full()) {
            return false;
        }

        if (group.is_newline()) {
            _current_line_consumed_bytes += group.consumed_bytes;
            _consumed_bytes += group.consumed_bytes;
            push_current_line();
            return true;
        }

        const std::size_t width = group_width(group, _text_box);
        if (should_start_new_line(width)) {
            push_current_line();
            if (page_full()) {
                return false;
            }
        }

        _current_line_consumed_bytes += group.consumed_bytes;
        _consumed_bytes += group.consumed_bytes;

        if (_current_groups.empty() && group.is_space()) {
            return true;
        }

        _cursor.x += static_cast<int16_t>(width);
        _current_groups.push_back(std::move(group));
        return true;
    }

    TextPage finish()
    {
        if (!_current_groups.empty() || _current_line_consumed_bytes > 0) {
            push_current_line();
        }

        return TextPage{std::move(_lines), _consumed_bytes};
    }

    bool page_full() const
    {
        if (line_height() <= 0) {
            return true;
        }

        return _cursor.y > _text_box.bottom();
    }

    int16_t line_height() const
    {
        const GFXfont* font = _text_box.font();
        if (font == nullptr) {
            return 0;
        }

        return static_cast<int16_t>(_text_box.textSize()) *
               static_cast<int16_t>(font->yAdvance);
    }

    bool should_start_new_line(const std::size_t width) const
    {
        return !_current_groups.empty() &&
               _text_box.width() > 0 &&
               static_cast<std::size_t>(_cursor.x) + width > static_cast<std::size_t>(_text_box.width());
    }

    void push_current_line()
    {
        trim_trailing_spaces();
        _lines.push_back(finalize_line(std::move(_current_groups), _current_line_consumed_bytes));
        _cursor.y += line_height();

        _current_groups = {};
        _current_line_consumed_bytes = 0;
        _cursor.x = 0;
    }

    void trim_trailing_spaces()
    {
        while (!_current_groups.empty() && _current_groups.back().is_space()) {
            _current_groups.pop_back();
        }
    }

    Line finalize_line(std::vector<WordGroup> logical_groups, const std::size_t consumed_bytes) const
    {
        if (logical_groups.empty()) {
            return Line{{}, consumed_bytes};
        }

        for (std::size_t i = 0; i < logical_groups.size();) {
            if (logical_groups[i].context_direction == _base_direction) {
                ++i;
                continue;
            }

            const std::size_t reverse_begin = i;
            while (i < logical_groups.size() &&
                   logical_groups[i].context_direction != _base_direction) {
                ++i;
            }

            std::reverse(logical_groups.begin() + reverse_begin, logical_groups.begin() + i);
        }

        if (_base_direction == Direction::LTR) {
            std::reverse(logical_groups.begin(), logical_groups.end());
        }

        Line line{{}, consumed_bytes};
        for (const WordGroup& group : logical_groups) {
            append_group_tokens(line, group);
        }

        return line;
    }

    void append_group_tokens(Line& line, const WordGroup& group) const
    {
        if (group.context_direction == Direction::LTR) {
            for (auto token = group.tokens.rbegin(); token != group.tokens.rend(); ++token) {
                line.tokens.push_back(*token);
            }
            return;
        }

        for (const ResolvedToken& token : group.tokens) {
            line.tokens.push_back(token);
        }
    }

private:
    const TextBox& _text_box;
    Direction _base_direction;
    std::vector<Line> _lines;
    std::vector<WordGroup> _current_groups;
    Vector2 _cursor;
    std::size_t _current_line_consumed_bytes = 0;
    std::size_t _consumed_bytes = 0;
};

} // namespace

std::vector<Line> PageSerializer::serialize_lines(
    std::vector<ResolvedToken> tokens,
    const TextBox& text_box,
    const Direction base_direction
)
{
    return serialize(std::move(tokens), text_box, base_direction).lines;
}

TextPage PageSerializer::serialize(
    std::vector<ResolvedToken> tokens,
    const TextBox& text_box,
    const Direction base_direction
)
{
    std::vector<WordGroup> groups = build_word_groups(std::move(tokens), base_direction);
    return PageSerializerRunner(text_box, base_direction).serialize(std::move(groups));
}
