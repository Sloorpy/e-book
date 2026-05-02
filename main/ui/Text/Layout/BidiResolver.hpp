#pragma once

#include "Text/Layout/TextToken.hpp"

class BidiResolver final {
public:
    static std::vector<ResolvedToken> resolve(std::vector<TextToken> tokens, const Direction base_direction);

private:
    BidiResolver(std::vector<TextToken> tokens, const Direction base_direction);

private:
    std::vector<ResolvedToken> run();

private:
    void resolve_token(TextToken& token);
    Direction resolve_direction(const TextToken& token) const;
    bool has_pending_neutrals() const;
    void flush_pending_neutrals(const Direction direction);
    bool is_neutral(const TextToken& token) const;

private:
    std::vector<TextToken> _tokens;
    std::vector<ResolvedToken> _resolved;
    Direction _base_direction;
    Direction _previous_strong;
    std::size_t _pending_neutral_start = 0;
    bool _has_previous_strong = false;
};
