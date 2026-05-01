#include "Text/Layout/BidiResolver.hpp"

#include <utility>

ResolvedTokens BidiResolver::resolve(TextTokens tokens, Direction base_direction)
{
    ResolvedTokens resolved;
    resolved.reserve(tokens.size());

    for (TextToken& token : tokens) {
        const Direction direction = resolve_direction(token, base_direction);
        resolved.push_back(ResolvedToken{std::move(token), direction});
    }

    return resolved;
}

Direction BidiResolver::resolve_direction(const TextToken& token, Direction base_direction)
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
            // TODO: Replace this fallback with contextual neutral resolution.
            return base_direction;
    }

    return base_direction;
}
