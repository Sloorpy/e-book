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
    bool has_next() const;
    bool has_prev() const;
    std::vector<uint8_t> next();
    std::vector<uint8_t> prev();

private:
    uint64_t previous_page_offset() const;
private:
    static constexpr size_t BUFFER_SIZE = 2048;
    const uint16_t _chapter_num;
    const uint64_t _chapter_size;
    std::weak_ptr<BookFs> _book_fs;
    uint64_t _current_offset;
};
