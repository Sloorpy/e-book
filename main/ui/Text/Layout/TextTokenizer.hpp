#pragma once

#include "Text/Layout/TextToken.hpp"
#include <cstdint>
#include <vector>

class TextTokenizer final {
public:
    // Converts serialized font bytes into owned text tokens.
    // Keep this function as the only tokenizer entry point so BookString/TextBox
    // do not need to know how token boundaries are detected.
    static std::vector<TextToken> tokenize(const std::vector<uint8_t>& bytes);

private:
    static TokenKind token_kind(const uint8_t byte);
    static TextToken next_token(std::vector<uint8_t>::const_iterator& current, std::vector<uint8_t>::const_iterator end);
    static std::vector<uint8_t>::const_iterator consume_token(
        std::vector<uint8_t>::const_iterator current,
        std::vector<uint8_t>::const_iterator end,
        TokenKind kind
    );
    static bool can_extend_token(TokenKind current_kind, TokenKind next_kind);
    static bool is_inner_sign(uint8_t byte);
    static bool can_consume_inner_sign(
        std::vector<uint8_t>::const_iterator sign,
        std::vector<uint8_t>::const_iterator end,
        TokenKind kind
    );
};
