#include "Text/Layout/TextTokenizer.hpp"
#include "Text/Support/TextHelper.hpp"

std::vector<TextToken> TextTokenizer::tokenize(const std::vector<uint8_t>& bytes)
{
    std::vector<TextToken> tokens;
    tokens.reserve(bytes.size());

    std::vector<uint8_t>::const_iterator current = bytes.begin();
    while (current != bytes.end()) {
        tokens.push_back(next_token(current, bytes.end()));
    }

    return tokens;
}

TokenKind TextTokenizer::token_kind(const uint8_t byte)
{
    if (TextHelper::is_newline_byte(byte)) {
        return TokenKind::Newline;
    }
    if (byte == ' ') {
        return TokenKind::Space;
    }
    if (TextHelper::is_hebrew_char(byte)) {
        return TokenKind::HebrewWord;
    }
    if (TextHelper::is_english_char(byte)) {
        return TokenKind::EnglishWord;
    }
    if (TextHelper::is_numeric_char(byte)) {
        return TokenKind::Number;
    }
    if (TextHelper::is_sign_char(byte)) {
        return TokenKind::Sign;
    }

    return TokenKind::Unknown;
}

TextToken TextTokenizer::next_token(
    std::vector<uint8_t>::const_iterator& current,
    std::vector<uint8_t>::const_iterator end
) {
    const TokenKind kind = token_kind(*current);
    std::vector<uint8_t>::const_iterator token_end = consume_token(current, end, kind);
    TextToken token{std::vector<uint8_t>(current, token_end), kind};
    current = token_end;
    return token;
}

std::vector<uint8_t>::const_iterator TextTokenizer::consume_token(
    std::vector<uint8_t>::const_iterator current,
    std::vector<uint8_t>::const_iterator end,
    TokenKind kind
) {
    std::vector<uint8_t>::const_iterator token_end = current + 1;

    if (kind == TokenKind::Newline) {
        if (token_end != end && (*current == '\r' && *token_end == '\n')) {
            ++token_end;
        }
        return token_end;
    }

    while (token_end != end) {
        const TokenKind next_kind = token_kind(*token_end);

        if ( !can_extend_token(kind, next_kind) &&
             !can_consume_inner_sign(token_end, end, kind)) {
            break;
        }

        ++token_end;
    }

    return token_end;
}

bool TextTokenizer::can_extend_token(TokenKind current_kind, TokenKind next_kind)
{
    return current_kind == next_kind && current_kind != TokenKind::Newline;
}

bool TextTokenizer::can_consume_inner_sign(
    std::vector<uint8_t>::const_iterator sign,
    std::vector<uint8_t>::const_iterator end,
    TokenKind kind
) {
    std::vector<uint8_t>::const_iterator next = sign + 1;

    if (next == end) {
        return false;
    }

    if (is_letter_inner_sign(*sign) &&
        (kind == TokenKind::HebrewWord || kind == TokenKind::EnglishWord)) {
        return token_kind(*next) == kind;
    }

    if (is_number_inner_sign(*sign) && kind == TokenKind::Number) {
        return token_kind(*next) == kind;
    }

    return false;
}

bool TextTokenizer::is_letter_inner_sign(const uint8_t byte)
{
    return byte == '\'' || byte == '/' || byte == '-';
}

bool TextTokenizer::is_number_inner_sign(const uint8_t byte)
{
    return byte == '.' || byte == ':' || byte == ',' || byte == '/';
}
