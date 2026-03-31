#include "Book.hpp"
#include "HebrewHelper.hpp"
#include "Fonts/hebEng5x7avia.h"
#include "Display.hpp"
#include <esp_log.h>

static constexpr char TAG[] = "Book";

static constexpr uint16_t MIDDLE_X = 270;
static constexpr uint16_t MIDDLE_Y = 75;
static constexpr uint16_t BOTTOM_X = 300;
static constexpr uint16_t BOTTOM_Y = 375;

Book::Book(std::shared_ptr<Display> display, const std::string_view& book_name, std::shared_ptr<SDManager> sd_reader) : 
    _page_manager(book_name, std::move(sd_reader)), 
    _text_box(create_text_box(display)),
    _current_chapter{"", "", 0, 0}
{}


void Book::display_title() const
{
    _text_box->setTextColor(static_cast<uint8_t>(Color::BLACK));
    _text_box->setFont(&hebEng5x7avia);

    _text_box->setTextSize(5);
    _text_box->setCursor(290, 40);
    _text_box->printHebrew(get_title().c_str());

    _text_box->setTextSize(2);
    _text_box->setCursor(300, 380);
    _text_box->printHebrew(get_author().c_str());
}

void Book::no_more_pages() const
{
    _text_box->setFont(&hebEng5x7avia);
    _text_box->setTextSize(3);
    _text_box->setTextColor(static_cast<uint8_t>(Color::BLACK));
    _text_box->printHebrew("נגמרו העמודים :)");
}

void Book::read_page()
{
    _text_box->resetCursor();

    if (_current_chapter.pages_offset >= _current_chapter.pages.length()) {
        const uint16_t next_chapter = _current_chapter.num + 1;
        if (next_chapter <= _page_manager.chapter_count()) {
            _current_chapter = _page_manager.load_chapter(next_chapter);
            display_chapter_title();
        }
        return;
    }

    const std::string curr_text = _current_chapter.pages.substr(_current_chapter.pages_offset);
    const size_t bytes_written = _text_box->printHebrew(curr_text.c_str());
    _current_chapter.pages_offset += bytes_written;
}


void Book::prev_page()
{

}

void Book::display_chapter_title() const
{ 
    _text_box->setTextColor(static_cast<uint8_t>(Color::BLACK));
    _text_box->setFont(&hebEng5x7avia);

    _text_box->setTextSize(8);
    _text_box->setCursor(180, 80);
    _text_box->printHebrew(std::to_string(_current_chapter.num).c_str());

    _text_box->setTextSize(3);
    _text_box->setCursor(PAGE_WIDTH, 240);
    _text_box->printHebrew(_page_manager.get_chapter_title(_current_chapter.num).c_str());
    _text_box->setTextSize(2);
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

std::unique_ptr<TextBox> Book::create_text_box(std::shared_ptr<Display> display)
{
    static constexpr int16_t LEFT = 0;
    static constexpr int16_t TOP = 2;
    static constexpr int16_t RIGHT = Book::PAGE_WIDTH;
    static constexpr int16_t BOTTOM = Book::PAGE_HEIGHT;
    
    return std::make_unique<TextBox>(
        display,
        LEFT,
        TOP,
        RIGHT, 
        BOTTOM, 
        WritingDirection::RTL
    );
}
