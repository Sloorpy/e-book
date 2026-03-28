#pragma once
#include "File.hpp"
#include "PageManager.hpp"
#include "Display.hpp"

#include "Adafruit_GFX.h"
#include <string_view>
#include <cstdint>

struct ReadingState {
    uint16_t chapter;
    size_t page_offset;
};

class Book final {
public:
    explicit Book(const std::string_view& book_name, std::shared_ptr<SDManager> sd_reader, const Display& display);
    ~Book() = default;

public:
    std::string get_page();
    void next_page();
    void prev_page();
    std::string get_title();
    std::string get_author();

public:
    bool has_next_page() const;
    bool has_prev_page() const;
    ReadingState get_state() const;

private:
    PageManager _page_manager;
    std::vector<Chapter> _chapters;
    const Display& _display;
    ReadingState _state;

private:
    static constexpr uint16_t PAGE_WIDTH = 300;
    static constexpr uint16_t PAGE_HEIGHT = 400;

private:
    static constexpr uint32_t CHAPTER_BUFFER_SIZE = 2;
};
