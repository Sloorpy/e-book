#include "Text/Layout/TextLayout.hpp"

#include "Text/Layout/BidiResolver.hpp"
#include "Text/Layout/LineBreaker.hpp"
#include "Text/Layout/TextTokenizer.hpp"

#include <utility>

std::vector<Line> TextLayout::build_lines(
    const std::vector<uint8_t>& serialized_bytes,
    Direction base_direction,
    const TextBox& text_box
) {
    std::vector<TextToken> tokens = TextTokenizer::tokenize(serialized_bytes);
    std::vector<ResolvedToken> resolved = BidiResolver::resolve(std::move(tokens), base_direction);
    return PageSerializer::serialize(std::move(resolved), text_box).lines;
}

TextPage TextLayout::build_page(
    const std::vector<uint8_t>& serialized_bytes,
    const Direction base_direction,
    const TextBox& text_box
) {
    std::vector<TextToken> page_tokens;
    PageSerializer page_serializer(text_box);

    for (TextTokenizer::Iterator iterator(serialized_bytes); iterator.has_next(); iterator.next()) {
        TextToken token = iterator.value();
        ResolvedToken measuring_token{TextToken{token.bytes, token.kind}, base_direction};

        if (!page_serializer.consume_token(std::move(measuring_token))) {
            break;
        }

        page_tokens.push_back(std::move(token));
    }

    std::vector<ResolvedToken> resolved = BidiResolver::resolve(std::move(page_tokens), base_direction);
    return PageSerializer::serialize(std::move(resolved), text_box);
}
