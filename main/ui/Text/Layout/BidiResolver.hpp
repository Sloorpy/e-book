#pragma once

#include "Text/Layout/TextToken.hpp"

#include <optional>

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
    void flush_pending_neutrals(const Direction next_strong_direction);
    void flush_trailing_neutrals();
    Direction resolve_neutral_run_direction(const Direction next_strong_direction) const;
    std::optional<Direction> resolve_single_sign_neutral_direction(
        const std::size_t begin,
        const std::size_t end,
        const Direction next_strong_direction
    ) const;
    std::size_t resolve_leading_attached_signs(const std::size_t begin, const std::size_t end);
    std::size_t resolve_trailing_attached_signs(
        const std::size_t begin,
        const std::size_t end,
        const Direction next_strong_direction
    );
    bool has_previous_strong() const;
    bool is_neutral(const TextToken& token) const;
    bool is_opening_sign(const TextToken& token) const;
    bool is_opening_sign(const uint8_t ch) const;

private:
    std::vector<TextToken> _tokens;
    std::vector<ResolvedToken> _resolved;
    Direction _base_direction;
    Direction _previous_strong;
    std::size_t _pending_neutral_start = 0;
};
