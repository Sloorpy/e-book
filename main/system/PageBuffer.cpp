#include "PageBuffer.hpp"
#include <stdexcept>

PageBuffer::PageBuffer(std::weak_ptr<BookFs> book_fs, const uint16_t chapter_num) :
    _chapter_num(chapter_num),
    _chapter_size(book_fs.lock()->chapter_size(chapter_num)),
    _book_fs(book_fs)
{
}

std::vector<uint8_t> PageBuffer::read_chunk(const uint64_t start, const uint64_t end)
{
    if (end <= start) {
        throw std::runtime_error("You cannot read from end to start");
    }

    std::shared_ptr<BookFs> book = _book_fs.lock();
    const std::vector<uint8_t> bytes = book->read_page(_chapter_num, end - start, start);
    return bytes;
}

uint64_t PageBuffer::size()
{
    return _chapter_size;
}
