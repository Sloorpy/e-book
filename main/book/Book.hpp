#pragma once
#include "File.hpp"
#include "PageManager.hpp"
#include "Text/TextBox.hpp"
#include "Display.hpp"

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
    void read_page();
    void reset_book();

public:
    bool has_next_page() const;
    bool has_prev_page() const;

private:
    void render_current_view();
    void display_title();
    void no_more_pages();
    size_t print_page();

private:
    void handle_chapter_title();
    void display_header();
    void load_chapter_save();
    size_t current_page_size() const;

private:
    void draw_cover(const uint16_t x, const uint16_t y);
    BookViewState handle_next_chapter();
    BookViewState handle_prev_chapter();
    BookViewState step_back_within_loaded_chapter();
    void build_page_indices(size_t end_offset);

private:
    const GFXfont* get_font() const;

private:
    TextBox make_header_text_box() const;
    TextBox make_page_text_box() const;
    TextBox make_text_box(int16_t left, int16_t top, int16_t right, int16_t bottom, uint16_t text_size) const;
    StateInfo current_state() const;

private:
    PageManager _page_manager;
    Chapter _current_chapter;
    BookViewState _view_state;

private:
    std::shared_ptr<Display> _display;

private:
    static constexpr uint16_t PAGE_WIDTH = 300;
    static constexpr uint16_t PAGE_HEIGHT = 400;
    static constexpr uint16_t BITMAP_WIDTH = 160;
    static constexpr uint16_t BITMAP_HEIGHT = 210;
};