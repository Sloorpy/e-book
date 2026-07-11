#pragma once
#include "BookFs.hpp"

#include <memory>
#include <array> 
#include <vector>

class PageBuffer final {
public:
    explicit PageBuffer(std::weak_ptr<BookFs> book_fs, const uint16_t chapter_num);
    ~PageBuffer() = default;

public:
    std::vector<uint8_t> read_chunk(const uint64_t start, const uint64_t end);
    uint64_t size();

private:
    //static constexpr size_t BUFFER_SIZE = 2048;
    //uint64_t _current_offset;
    const uint16_t _chapter_num;
    const uint64_t _chapter_size;
    std::weak_ptr<BookFs> _book_fs;
};
