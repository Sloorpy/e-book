#include "Text/Layout/TextLayout.hpp"

#include "Text/Layout/BidiResolver.hpp"
#include "Text/Layout/LineBreaker.hpp"
#include "Text/Layout/TextTokenizer.hpp"

#include <utility>

LayoutLines TextLayout::build_lines(
    const std::vector<uint8_t>& serialized_bytes,
    Direction base_direction,
    const TextBox& text_box
) {
    TextTokens tokens = TextTokenizer::tokenize(serialized_bytes);
    ResolvedTokens resolved = BidiResolver::resolve(std::move(tokens), base_direction);
    return LineBreaker::break_lines(std::move(resolved), text_box);
}
