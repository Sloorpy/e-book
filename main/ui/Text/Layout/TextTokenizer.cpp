#include "Text/Layout/TextTokenizer.hpp"

std::vector<TextToken> TextTokenizer::tokenize(const std::vector<uint8_t>& bytes)
{
    std::vector<TextToken>  tokens;

    // TODO: Implement tokenization here.
    //
    // Suggested flow:
    // 1. Walk the input with STL iterators.
    // 2. Build TextToken values that own only their token bytes.
    // 3. Classify each token as HebrewWord, EnglishWord, Number, Sign, Space,
    //    Newline, or Unknown.
    // 4. Keep spaces and newlines as tokens so page-size accounting remains
    //    accurate after layout.
    //
    // This placeholder keeps the class compilable while the current BookString
    // path remains active.
    if (!bytes.empty()) {
        tokens.push_back(TextToken{bytes, TokenKind::Unknown});
    }

    for (const uint8_t byte: bytes) {
        
    }

    return tokens;
}
