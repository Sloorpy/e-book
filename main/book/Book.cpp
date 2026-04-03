#include "Book.hpp"
#include "Text/TextHelper.hpp"
#include "Text/TextBox.hpp"
#include "Fonts/hebEng5x7avia.h"
#include "Display.hpp"
#include <esp_log.h>
#include <esp_timer.h>
#include <cstring>

static constexpr char TAG[] = "Book";

static constexpr uint16_t MIDDLE_X = 270;
static constexpr uint16_t MIDDLE_Y = 75;
static constexpr uint16_t BOTTOM_X = 300;
static constexpr uint16_t BOTTOM_Y = 375;
static const Chapter BASE_CHAPTER{{}, "", 0, 0};

Book::Book(std::shared_ptr<Display> display, const std::string_view& book_name) : 
    _page_manager(book_name),
    _current_chapter(BASE_CHAPTER),
    _page_indexs(),
    _display(display)
    {}


void Book::display_title()
{
    TextBox text_box = make_text_box(0, 2, PAGE_WIDTH, PAGE_HEIGHT);
    
    text_box.setTextColor(static_cast<uint8_t>(Color::BLACK));
    text_box.setFont(&hebEng5x7avia);
    const std::string title = get_title();
    const std::string author = get_author();
    text_box.setTextSize(5);
    text_box.setCursor(290, 40);
    text_box.print_hebrew(TextHelper::serialize_to_font_indices(title, &hebEng5x7avia));

    text_box.setTextSize(2);
    text_box.setCursor(300, 380);
    text_box.print_hebrew(TextHelper::serialize_to_font_indices(author, &hebEng5x7avia));

    static constexpr uint16_t bmp_y = 150;
    static uint16_t bmp_x = (PAGE_WIDTH - static_cast<int16_t>(BITMAP_WIDTH)) / 2;
    draw_cover(bmp_x,bmp_y);
}

void Book::no_more_pages()
{
    TextBox text_box = make_text_box(0, 2, PAGE_WIDTH, PAGE_HEIGHT);
    
    text_box.setFont(&hebEng5x7avia);
    text_box.setTextSize(3);
    text_box.setTextColor(static_cast<uint8_t>(Color::BLACK));
    const char* msg = "נגמרו העמודים :)";
    text_box.print_hebrew(TextHelper::serialize_to_font_indices(
        std::vector<uint8_t>(msg, msg + strlen(msg)), 
        &hebEng5x7avia
    ));
}

void Book::reset_book()
{
    _current_chapter = Chapter{{}, "", 0, 0};
}

void Book::read_page()
{
    if (_current_chapter.pages_offset >= _current_chapter.pages.size()) {
        const uint16_t next_chapter = _current_chapter.num + 1;
        if (next_chapter <= _page_manager.chapter_count()) {
            _current_chapter = _page_manager.load_chapter(next_chapter, get_font());
            display_chapter_title();
        }
        return;
    }

    TextBox text_box = make_text_box(0, 28, PAGE_WIDTH, PAGE_HEIGHT);
    text_box.setTextSize(2);
    _display->drawLine(0, 20, PAGE_WIDTH, 20, 0);
    
    const std::vector<uint8_t> curr_text(_current_chapter.pages.begin() + _current_chapter.pages_offset, _current_chapter.pages.end());
    //const size_t next_size = text_box.next_print_size(curr_text);
    const size_t bytes_written = text_box.print_hebrew(curr_text);
    _page_indexs.push(_current_chapter.pages_offset);
    _current_chapter.pages_offset += bytes_written;
}


void Book::prev_page()
{
    if (_current_chapter.pages_offset <= 0) {
        const uint16_t prev_chapter = _current_chapter.num - 1;
        if (prev_chapter <= 0) {
            _current_chapter = BASE_CHAPTER;
            display_title();
            return;
        }

        _current_chapter = _page_manager.load_chapter(prev_chapter, get_font());
        display_chapter_title();
        return;
    }
}

void Book::display_chapter_title()
{ 
    TextBox text_box = make_text_box(0, 2, PAGE_WIDTH, PAGE_HEIGHT);
    
    text_box.setTextColor(static_cast<uint8_t>(Color::BLACK));
    text_box.setFont(&hebEng5x7avia);

    text_box.setTextSize(8);
    text_box.setCursor(180, 80);
    std::string chapter_num_str = std::to_string(_current_chapter.num);
    text_box.print_hebrew(TextHelper::serialize_to_font_indices(
        std::vector<uint8_t>(chapter_num_str.begin(), chapter_num_str.end()), 
        &hebEng5x7avia
    ));

    text_box.setTextSize(3);
    text_box.setCursor(PAGE_WIDTH, 240);
    std::string chapter_title = _page_manager.get_chapter_title(_current_chapter.num);
    text_box.print_hebrew(TextHelper::serialize_to_font_indices(
        std::vector<uint8_t>(chapter_title.begin(), chapter_title.end()), 
        &hebEng5x7avia
    ));
    text_box.setTextSize(2);
}

std::string Book::get_title() const
{
    std::string cover = _page_manager.get_cover();
    size_t index = cover.find('\n');

    if (index == std::string::npos)
    {
        return cover;
    }

    return cover.substr(0, index);
}

std::string Book::get_author() const
{
    std::string cover = _page_manager.get_cover();
    size_t index = cover.find('\n');

    if (index == std::string::npos)
    {
        return cover;
    }

    return cover.substr(index + 1);
}

void Book::draw_cover(const uint16_t start_x, const uint16_t start_y)
{
    std::vector<uint8_t> bitmap = File("books/percy_2_heb/cover.bin").read_all_bytes();
    _display->drawRect(start_x, start_y, BITMAP_WIDTH, BITMAP_HEIGHT, 0);
    _display->drawBitmap(start_x, start_y, bitmap.data(), BITMAP_WIDTH, BITMAP_HEIGHT, static_cast<uint16_t>(Color::BLACK));
}

bool Book::has_next_page() const
{
    if (_current_chapter.pages_offset < _current_chapter.pages.size())
    {
        return true;
    }
    
    return _current_chapter.num < _page_manager.chapter_count();
}

bool Book::has_prev_page() const
{
    return false;
}

TextBox Book::make_text_box(int16_t left, int16_t top, int16_t right, int16_t bottom)
{
    return TextBox(
        _display,
        left,
        top,
        right, 
        bottom, 
        WritingDirection::RTL
    );
}

const GFXfont* Book::get_font() const
{
    return &hebEng5x7avia;
}
