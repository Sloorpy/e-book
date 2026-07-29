#include "PageIter.hpp"
#include <Text/Layout/TextLayout.hpp>

#include <stdexcept>
#include <algorithm>

PageIter::PageIter(std::unique_ptr<PageBuffer> buffer, std::unique_ptr<TextBox> tb_mock, const Direction direction) :
    _current_page(0),
    _page_indecies(initialize_page_indecies(*buffer, *tb_mock, direction)),
    _buffer(std::move(buffer)),
    _tb_mock(std::move(tb_mock)),
    _direction(direction)
{
}

bool PageIter::has_next() const
{
    return _current_page < _page_indecies.size();
}

bool PageIter::has_prev() const
{
    return _current_page > 0;
}

TextPage PageIter::next()
{
    if (!has_next()) {
        throw std::out_of_range("No next page.");
    }

    const PageRange& range = _page_indecies[_current_page++];
    std::vector<uint8_t> data = _buffer->read_chunk(range.start, range.end);
    return TextLayout::build_page(data, _direction, *_tb_mock);
}

TextPage PageIter::prev()
{
    if (!has_prev()) {
        throw std::out_of_range("No previous page.");
    }

    const PageRange& range = _page_indecies[--_current_page];
    std::vector<uint8_t> data = _buffer->read_chunk(range.start, range.end);
    return TextLayout::build_page(data, _direction, *_tb_mock);
}

std::vector<PageRange> PageIter::initialize_page_indecies(PageBuffer &buffer, TextBox &tb_mock, const Direction direction)
{
    std::vector<PageRange> indices;
    const uint32_t chapter_size = buffer.size();
    uint32_t offset = 0;

    static constexpr uint32_t BASE_BUFFER_SIZE = 512;
    uint32_t buffer_size = BASE_BUFFER_SIZE;
    while (offset < chapter_size) {
        std::vector<uint8_t> chunk = buffer.read_chunk(offset, offset + buffer_size);
        const TextPage page = TextLayout::build_page(chunk, direction, tb_mock);

        if (page.consumed_bytes == 0) {
            throw std::runtime_error("Failed to build indecies in PageIter::initialize_page_indecies");
        }
        
        if (page.consumed_bytes == buffer_size) {
            buffer_size += BASE_BUFFER_SIZE / 2;
            continue;
        }

        indices.push_back(PageRange{offset, std::min(offset + page.consumed_bytes, chapter_size)});
        offset += page.consumed_bytes;
    }
    return indices;
}
