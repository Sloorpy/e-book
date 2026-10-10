#include "PageBuffer.hpp"
#include <stdexcept>

PageBuffer::PageBuffer(std::weak_ptr<BookFs> book_fs, const uint16_t chapter_num) :
    _chapter_num(chapter_num),
    _chapter_size(static_cast<uint32_t>(book_fs.lock()->chapter_size(chapter_num))),
    _book_fs(book_fs)
{
}

std::vector<uint8_t> PageBuffer::read_chunk(const uint32_t start, const uint32_t end)
{
    if (end <= start) {
        throw std::runtime_error("You cannot read from end to start");
    }

    std::shared_ptr<BookFs> book = _book_fs.lock();
    const std::vector<uint8_t> bytes = book->read_page(
        _chapter_num,
        static_cast<size_t>(end - start),
        static_cast<uint64_t>(start)
    );
    return bytes;
}

uint32_t PageBuffer::size()
{
    return _chapter_size;
}
