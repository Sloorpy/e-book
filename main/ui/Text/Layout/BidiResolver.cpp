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

    if (has_pending_neutrals()) {
        flush_pending_neutrals(_has_previous_strong ? _previous_strong : _base_direction);
    }

    return std::move(_resolved);
}

void BidiResolver::resolve_token(TextToken& token)
{
    if (is_neutral(token)) {
        _resolved.push_back(ResolvedToken{std::move(token), _base_direction});
        return;
    }

    const Direction direction = resolve_direction(token);

    if (has_pending_neutrals()) {
        Direction neutral_direction = direction;

        if (_has_previous_strong) {
            neutral_direction = _previous_strong == direction ? direction : _base_direction;
        }

        flush_pending_neutrals(neutral_direction);
    }

    _resolved.push_back(ResolvedToken{std::move(token), direction});
    _previous_strong = direction;
    _has_previous_strong = true;
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

void BidiResolver::flush_pending_neutrals(const Direction direction)
{
    while (has_pending_neutrals()) {
        _resolved[_pending_neutral_start++].direction = direction;
    }
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
