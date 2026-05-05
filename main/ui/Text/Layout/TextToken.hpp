#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

enum class Direction : uint8_t {
    LTR,
    RTL
};

enum class TokenKind : uint8_t {
    HebrewWord,
    EnglishWord,
    Number,
    Sign,
    Space,
    Newline,
    Unknown
};

struct TextToken final {
    std::vector<uint8_t> bytes;
    TokenKind kind = TokenKind::Unknown;

    bool empty() const { return bytes.empty(); }
    std::size_t size() const { return bytes.size(); }
};

struct ResolvedToken final {
    TextToken token;
    Direction direction = Direction::LTR;
};

using Line = std::vector<ResolvedToken>;
