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
    resolve_neutral_run_boundaries(neutral_run_begin, neutral_run_end, next_strong_direction);

    const Direction neutral_run_direction = resolve_neutral_run_direction(next_strong_direction);
    for (std::size_t i = neutral_run_begin + 1; i + 1 < neutral_run_end; ++i) {
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

void BidiResolver::resolve_neutral_run_boundaries(
    const std::size_t begin,
    const std::size_t end,
    const Direction next_strong_direction
) {
    const Direction neutral_run_direction = resolve_neutral_run_direction(next_strong_direction);
    const std::size_t first_token = begin;
    const std::size_t last_token = end - 1;
    const bool has_multiple_tokens = first_token != last_token;

    _resolved[first_token].direction = neutral_run_direction;

    if (_resolved[first_token].token.kind == TokenKind::Sign &&
        has_previous_strong() && has_multiple_tokens) {
        _resolved[first_token].direction = _previous_strong;
    }

    if (!has_multiple_tokens) {
        return;
    }

    _resolved[last_token].direction = neutral_run_direction;

    if (_resolved[last_token].token.kind == TokenKind::Sign) {
        _resolved[last_token].direction = next_strong_direction;
    }
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
