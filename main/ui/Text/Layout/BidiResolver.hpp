#pragma once

#include "Text/Layout/TextToken.hpp"

class BidiResolver final {
public:
    // Assigns a drawing direction to each token. The input tokens are passed by
    // value intentionally so callers can move token ownership into this stage.
    static std::vector<ResolvedToken> resolve(std::vector<TextToken> tokens, Direction base_direction);

private:
    static Direction resolve_direction(const TextToken& token, Direction base_direction);

    // TODO: Add neutral-token handling here.
    // Signs, spaces, and newlines should not decide direction by themselves.
    // Resolve them from nearby strong tokens or fall back to base_direction.
};
