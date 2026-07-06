#pragma once
#include "BookFs.hpp"

#include <memory>
#include <array> 

class PageBuffer final {
public:
    explicit PageBuffer(std::weak_ptr<BookFs> book_fs, const uint16_t chapter_num);
    ~PageBuffer() = default;

public:
    bool has_next() const;
    bool has_prev() const;

private:
    static constexpr size_t BUFFER_SIZE = 2048;
    std::array<uint8_t, BUFFER_SIZE> _buffer;
    std::weak_ptr<BookFs> _book_fs;
    size_t current_offset;
};
