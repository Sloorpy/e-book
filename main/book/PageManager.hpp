#pragma once

#include "File.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

struct Chapter final {
    std::string pages;
    std::string chapter_title;
    size_t pages_offset;
    uint16_t num;
};

class PageManager final {
public:
    explicit PageManager(const std::string_view& book_name);

public:
    std::string get_cover() const;
    std::string get_chapter_title(const uint16_t chapter_num) const;

    Chapter load_chapter(const uint16_t chapter_num);
    uint16_t chapter_count() const;

private:
    std::string book_root_path() const;
    std::string book_file_path(const std::string_view& filename) const;
    std::string chapter_root_path(uint16_t chapter_num) const;
    std::string chapter_file_path(uint16_t chapter_num, const std::string_view& filename) const;

private:
    const std::string_view _book_name;
    uint16_t _chapters_count;
};
