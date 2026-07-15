#pragma once

#include "File.hpp"
#include "Text/Support/TextHelper.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <stack>
#include <gfxfont.h>

struct PageRange final {
    uint32_t start;
    uint32_t end;
};

struct Chapter final {
    std::stack<PageRange> page_indicies;
    std::vector<uint8_t> pages;
    std::string chapter_title;
    uint16_t num;
};

struct StateInfo final {
    uint16_t chapter_num;
    uint32_t index;
};

class BookFs final {
public:
    explicit BookFs(const std::string_view& book_name);

public:
    std::string get_title() const;
    std::string get_author() const;
    std::string get_chapter_title(const uint16_t chapter_num) const;
    std::vector<uint8_t> read_page(const uint16_t chapter_num, const size_t buffer_size, const uint64_t offset = 0);
    uint64_t chapter_size(const uint16_t chapter_num);

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
