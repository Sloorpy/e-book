#pragma once
#include "File.hpp"
#include "PageManager.hpp"
#include "TextBox.hpp"
#include "Display.hpp"

#include <memory>
#include <string_view>
#include <cstdint>

class Book final {
public:
    explicit Book(std::shared_ptr<Display> display, const std::string_view& book_name);
    ~Book() = default;

public:
    void read_page();
    void prev_page();

public:
    void display_title();
    void no_more_pages();
    
public:
    bool has_next_page() const;
    bool has_prev_page() const;

private:
    void display_chapter_title();
    std::string get_title() const;
    std::string get_author() const;
    void draw_cover(const uint16_t x, const uint16_t y);
    
private:
    TextBox make_text_box(int16_t left, int16_t top, int16_t right, int16_t bottom);

private:
    PageManager _page_manager;
    Chapter _current_chapter;

private:
    std::shared_ptr<Display> _display;

private:
    static constexpr uint16_t PAGE_WIDTH = 300;
    static constexpr uint16_t PAGE_HEIGHT = 400;
    static constexpr uint16_t BITMAP_WIDTH = 160;
    static constexpr uint16_t BITMAP_HEIGHT = 210;
};
