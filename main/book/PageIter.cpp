#include "PageIter.hpp"
#include <stdexcept>
PageIter::PageIter(std::unique_ptr<PageBuffer> buffer) :
    _current_page(0),
    _page_indecies(initialize_page_indecies(*buffer)),
    _buffer(std::move(buffer))
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
        throw std::runtime_error("Check 'has_next' before calling 'next'.");
    }

    const PageRange& range = _page_indecies[++_current_page];
    std::vector<uint8_t> data = _buffer->read_chunk(range.start, range.end);
}

TextPage PageIter::prev()
{
    if (!has_prev()) {
        throw std::runtime_error("Check 'has_prev' before calling 'prev'.");
    }

    const PageRange& range = _page_indecies[--_current_page];
    std::vector<uint8_t> data = _buffer->read_chunk(range.start, range.end);
}
