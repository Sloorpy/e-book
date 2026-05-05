#pragma once

#include "Text/Layout/TextToken.hpp"
#include <cstdint>
#include <vector>

class TextTokenizer final {
public:
    class Iterator final {
    public:
        explicit Iterator(const std::vector<uint8_t>& serialized_bytes);

        bool has_next() const;
        const TextToken& value() const;
        void next();

    private:
        void read_next();

    private:
        const std::vector<uint8_t>& _serialized_bytes;
        std::vector<uint8_t>::const_iterator _current;
        TextToken _value;
        bool _has_value = false;
    };

public:
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
    static bool can_consume_inner_sign(
        std::vector<uint8_t>::const_iterator sign,
        std::vector<uint8_t>::const_iterator end,
        TokenKind kind
    );
    static bool is_letter_inner_sign(const uint8_t byte);
    static bool is_number_inner_sign(const uint8_t byte);
};
