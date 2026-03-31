#pragma once
#include "File.hpp"
#include "PageManager.hpp"
#include "TextBox.hpp"

#include <string_view>
#include <cstdint>

class Book final {
public:
    explicit Book(std::shared_ptr<Display> display, const std::string_view& book_name, std::shared_ptr<SDManager> sd_reader);
    ~Book() = default;

public:
    void read_page();
    void prev_page();

public:
    void display_title() const;
    void no_more_pages() const;
    
public:
    bool has_next_page() const;
    bool has_prev_page() const;

private:
    void display_chapter_title() const;
    std::string get_title() const;
    std::string get_author() const;
    
private:
    static std::unique_ptr<TextBox> create_text_box(std::shared_ptr<Display> display);

private:
    PageManager _page_manager;
    std::unique_ptr<TextBox> _text_box;

private:
    Chapter _current_chapter;

public:
    static constexpr uint16_t PAGE_WIDTH = 300;
    static constexpr uint16_t PAGE_HEIGHT = 400;

private:
    static constexpr uint32_t CHAPTER_BUFFER_SIZE = 2;
};
