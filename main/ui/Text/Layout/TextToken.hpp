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

struct Line final {
    std::vector<ResolvedToken> tokens;
    std::size_t consumed_bytes = 0;

    bool empty() const { return tokens.empty(); }
    std::size_t size() const { return tokens.size(); }
};

struct TextPage final {
    std::vector<Line> lines;
    std::size_t consumed_bytes = 0;
};
