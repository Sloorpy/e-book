#include "Text/Layout/BidiResolver.hpp"

#include <utility>

std::vector<ResolvedToken> BidiResolver::resolve(std::vector<TextToken> tokens, const Direction base_direction)
{
    return BidiResolver(std::move(tokens), base_direction).run();
}

BidiResolver::BidiResolver(std::vector<TextToken> tokens, const Direction base_direction)
    : _tokens(std::move(tokens))
    , _base_direction(base_direction)
    , _previous_strong(base_direction)
{
    _resolved.reserve(_tokens.size());
}

std::vector<ResolvedToken> BidiResolver::run()
{
    for (TextToken& token : _tokens) {
        resolve_token(token);
    }

    flush_trailing_neutrals();

    return std::move(_resolved);
}

void BidiResolver::resolve_token(TextToken& token)
{
    if (is_neutral(token)) {
        _resolved.push_back(ResolvedToken{std::move(token), _base_direction});
        return;
    }

    const Direction direction = resolve_direction(token);

    flush_pending_neutrals(direction);

    _resolved.push_back(ResolvedToken{std::move(token), direction});
    _previous_strong = direction;
    _pending_neutral_start = _resolved.size();
}

Direction BidiResolver::resolve_direction(const TextToken& token) const
{
    switch (token.kind) {
        case TokenKind::HebrewWord:
            return Direction::RTL;
        case TokenKind::EnglishWord:
        case TokenKind::Number:
            return Direction::LTR;
        case TokenKind::Sign:
        case TokenKind::Space:
        case TokenKind::Newline:
        case TokenKind::Unknown:
            return _base_direction;
    }

    return _base_direction;
}

bool BidiResolver::has_pending_neutrals() const
{
    return _pending_neutral_start < _resolved.size();
}

void BidiResolver::flush_pending_neutrals(const Direction next_strong_direction)
{
    if (!has_pending_neutrals()) {
        return;
    }

    const std::size_t neutral_run_begin = _pending_neutral_start;
    const std::size_t neutral_run_end = _resolved.size();

    const std::optional<Direction> single_sign_direction = resolve_single_sign_neutral_direction(
        neutral_run_begin,
        neutral_run_end,
        next_strong_direction
    );

    if (single_sign_direction.has_value()) {
        _resolved[neutral_run_begin].direction = *single_sign_direction;
        _pending_neutral_start = neutral_run_end;
        return;
    }

    const Direction neutral_run_direction = resolve_neutral_run_direction(next_strong_direction);
    const std::size_t middle_begin = resolve_leading_attached_signs(neutral_run_begin, neutral_run_end);
    const std::size_t middle_end = resolve_trailing_attached_signs(
        middle_begin,
        neutral_run_end,
        next_strong_direction
    );

    for (std::size_t i = middle_begin; i < middle_end; ++i) {
        _resolved[i].direction = neutral_run_direction;
    }

    _pending_neutral_start = neutral_run_end;
}

void BidiResolver::flush_trailing_neutrals()
{
    const Direction direction = has_previous_strong() ? _previous_strong : _base_direction;

    while (has_pending_neutrals()) {
        _resolved[_pending_neutral_start++].direction = direction;
    }
}

Direction BidiResolver::resolve_neutral_run_direction(const Direction next_strong_direction) const
{
    if (!has_previous_strong() || _previous_strong == next_strong_direction) {
        return next_strong_direction;
    }

    return _base_direction;
}

std::optional<Direction> BidiResolver::resolve_single_sign_neutral_direction(
    const std::size_t begin,
    const std::size_t end,
    const Direction next_strong_direction
) const
{
    if (begin + 1 != end || _resolved[begin].token.kind != TokenKind::Sign) {
        return std::nullopt;
    }

    if (has_previous_strong() && _previous_strong != next_strong_direction) {
        return is_opening_sign(_resolved[begin].token)
            ? next_strong_direction
            : _previous_strong;
    }

    return resolve_neutral_run_direction(next_strong_direction);
}

std::size_t BidiResolver::resolve_leading_attached_signs(const std::size_t begin, const std::size_t end)
{
    if (!has_previous_strong()) {
        return begin;
    }

    std::size_t current = begin;

    while (current < end && _resolved[current].token.kind == TokenKind::Sign) {
        if (is_opening_sign(_resolved[current].token)) {
            break;
        }

        _resolved[current].direction = _previous_strong;
        ++current;
    }

    return current;
}

std::size_t BidiResolver::resolve_trailing_attached_signs(
    const std::size_t begin,
    const std::size_t end,
    const Direction next_strong_direction
)
{
    std::size_t current = end;

    while (begin < current &&
           _resolved[current - 1].token.kind == TokenKind::Sign &&
           is_opening_sign(_resolved[current - 1].token)) {
        --current;
        _resolved[current].direction = next_strong_direction;
    }

    return current;
}

bool BidiResolver::has_previous_strong() const
{
    return _pending_neutral_start > 0;
}

bool BidiResolver::is_neutral(const TextToken& token) const
{
    switch (token.kind) {
        case TokenKind::Sign:
        case TokenKind::Space:
        case TokenKind::Newline:
        case TokenKind::Unknown:
            return true;
        case TokenKind::HebrewWord:
        case TokenKind::EnglishWord:
        case TokenKind::Number:
            return false;
    }

    return true;
}

bool BidiResolver::is_opening_sign(const TextToken& token) const
{
    return !token.bytes.empty() && is_opening_sign(token.bytes.front());
}

bool BidiResolver::is_opening_sign(const uint8_t ch) const
{
    return ch == '(' || ch == '[' || ch == '{';
}
