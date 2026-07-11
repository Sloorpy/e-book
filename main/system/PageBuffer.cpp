#include "PageBuffer.hpp"
#include <stdexcept>

PageBuffer::PageBuffer(std::weak_ptr<BookFs> book_fs, const uint16_t chapter_num) :
    _chapter_num(chapter_num),
    _chapter_size(book_fs.lock()->chapter_size(chapter_num)),
    _book_fs(book_fs),
    _current_offset(0)
{
}

bool PageBuffer::has_next() const
{
    return _current_offset < _chapter_size;
}

bool PageBuffer::has_prev() const
{
    return _current_offset > BUFFER_SIZE;
}

std::vector<uint8_t> PageBuffer::next()
{
    if (!has_next()) {
        throw std::runtime_error("PageBuffer iterator is at end");
    }

    std::shared_ptr<BookFs> book = _book_fs.lock();
    const std::vector<uint8_t> bytes = book->read_page(_chapter_num, BUFFER_SIZE, _current_offset);

    _current_offset += bytes.size();
    if (bytes.size() != BUFFER_SIZE && has_next()) {
        throw std::runtime_error("PageBuffer fail to read BUFFER_SIZE bytes");
    }

    return bytes;
}

std::vector<uint8_t> PageBuffer:: prev()
{
    if (!has_prev()) {
        throw std::runtime_error("PageBuffer iterator is at start");
    }

    std::shared_ptr<BookFs> book = _book_fs.lock();

    const uint64_t previous_page = previous_page_offset();
    const std::vector<uint8_t> bytes = book->read_page(_chapter_num, BUFFER_SIZE, previous_page);

    _current_offset = previous_page + bytes.size();
    if (bytes.size() != BUFFER_SIZE && has_prev()) {
        throw std::runtime_error("PageBuffer fail to read BUFFER_SIZE bytes");
    }

    return bytes;
}

uint64_t PageBuffer::previous_page_offset() const
{
    if (_current_offset >= BUFFER_SIZE * 2) {
        return _current_offset - BUFFER_SIZE * 2;
    }
    return 0;
}
