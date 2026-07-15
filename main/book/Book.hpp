#pragma once
#include "File.hpp"
#include "BookFs.hpp"
#include "Text/Rendering/TextBox.hpp"
#include "Display.hpp"

#include <cstddef>
#include <memory>
#include <string_view>
#include <cstdint>

enum class BookViewState: uint8_t {
    BOOK_TITLE = 0,
    CHAPTER_TITLE,
    PAGE,
    FINISHED
};

class Book final {
public:
    explicit Book(std::shared_ptr<Display> display, const std::string_view& book_name);
    ~Book() = default;

public:
    void next_page();
    void prev_page();
    void curr_page();
    void reset_book();

public:
    bool has_next_page() const;
    bool has_prev_page() const;

private:
    void render_current_view();
    void render_and_save();

private:
    void display_title();
    void display_chapter_title();
    void display_current_page();
    void display_header();
    void no_more_pages();

private:
    void load_chapter_save();
    void first_chapter_title();
    void load_all_pages();
    void prev_chapter_last_page();

private:
    void draw_cover(const uint16_t x, const uint16_t y);
    void build_page_indices(uint32_t end_offset);
    static uint32_t next_print_size_from(const std::vector<uint8_t>& pages, uint32_t page_start, TextBox& text_box);
    BookViewState handle_next_chapter();
    BookViewState handle_prev_chapter();

private:
    const GFXfont* get_font() const;

private:
    TextBox make_header_text_box(const int16_t start_x, const int16_t end_x) const;
    TextBox make_page_text_box() const;
    TextBox make_text_box(int16_t left, int16_t top, int16_t right, int16_t bottom, uint16_t text_size) const;
    StateInfo current_state() const;

private:
    BookFs _page_manager;
    Chapter _current_chapter;
    BookViewState _view_state;

private:
    std::shared_ptr<Display> _display;

private:
    static constexpr uint16_t PAGE_WIDTH = 300;
    static constexpr uint16_t PAGE_HEIGHT = 400;
    static constexpr uint16_t BITMAP_WIDTH = 160;
    static constexpr uint16_t BITMAP_HEIGHT = 210;
    static constexpr size_t INITIAL_PAGE_WINDOW_SIZE = 2048;
};
