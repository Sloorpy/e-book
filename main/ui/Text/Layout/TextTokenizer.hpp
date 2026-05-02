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

    // TODO: Add iterator-based helpers that consume one logical token at a time.
    // The tokenizer should split signs from words unless a sign belongs inside
    // a token, for example "3.14", "12:30", "don't", or "A/B".
};
