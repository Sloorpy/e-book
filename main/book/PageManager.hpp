#pragma once

#include "File.hpp"
#include "Text/TextHelper.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <stack>
#include <gfxfont.h>

struct PageRange final {
    size_t start;
    size_t end;
};

struct Chapter final {
    std::stack<PageRange> page_indicies;
    std::vector<uint8_t> pages;
    std::string chapter_title;
    uint16_t num;
};

struct StateInfo final {
    uint16_t chapter_num;
    size_t index;
};

class PageManager final {
public:
    explicit PageManager(const std::string_view& book_name);

public:
    std::string get_title() const;
    std::string get_author() const;
    std::string get_chapter_title(const uint16_t chapter_num) const;

    Chapter load_chapter(const uint16_t chapter_num, const GFXfont* font);
    uint16_t chapter_count() const;
    StateInfo load_state(const uint16_t state_num=1);
    void save_state(const StateInfo state, const uint16_t state_num=1);
    std::vector<uint8_t> cover_bitmap();
    
private:
    std::string book_root_path() const;
    std::string book_file_path(const std::string_view& filename) const;
    std::string chapter_root_path(uint16_t chapter_num) const;
    std::string chapter_file_path(uint16_t chapter_num, const std::string_view& filename) const;

private:
    const std::string_view _book_name;
    uint16_t _chapters_count;
};
