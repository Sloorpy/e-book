#include "Book.hpp"
#include "Text/TextHelper.hpp"
#include "Text/TextBox.hpp"
#include "Fonts/hebEng5x7avia.h"
#include "Display.hpp"
#include <esp_log.h>
#include <esp_timer.h>
#include <cstring>

static constexpr char TAG[] = "Book";
static constexpr bool CENETER_TEXT = true;

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
    static constexpr Vector2 TITLE_POSITION{0, 40};
    static constexpr Vector2 AUTHOR_POSITION{0, 380};
    static constexpr uint16_t TITLE_TEXT_SIZE = 5;
    static constexpr uint16_t AUTHOR_TEXT_SIZE = 2;
    
    const std::string title = _page_manager.get_title();
    const std::string author = _page_manager.get_author();
    
    TextBox title_tb = make_text_box(TITLE_POSITION.x, TITLE_POSITION.y, PAGE_WIDTH, PAGE_HEIGHT, TITLE_TEXT_SIZE);
    TextBox author_tb = make_text_box(AUTHOR_POSITION.x, AUTHOR_POSITION.y, PAGE_WIDTH, PAGE_HEIGHT, AUTHOR_TEXT_SIZE);

    title_tb.print_hebrew(TextHelper::serialize_to_font_indices(title, get_font()), CENETER_TEXT);

    author_tb.print_hebrew(TextHelper::serialize_to_font_indices(author, get_font()));
    
    static const Vector2 bitmap_position{(PAGE_WIDTH - static_cast<int16_t>(BITMAP_WIDTH)) / 2, 150};
    draw_cover(bitmap_position.x, bitmap_position.y);
}

void Book::no_more_pages()
{
    static constexpr Vector2 TEXT_POSITION{0, 50};
    static constexpr uint16_t TEXT_SIZE = 3;

    TextBox text_box = make_text_box(TEXT_POSITION.x, TEXT_POSITION.y, PAGE_WIDTH, PAGE_HEIGHT, TEXT_SIZE);
    
    const char* msg = "נגמרו העמודים :)";
    text_box.print_hebrew(TextHelper::serialize_to_font_indices(msg, get_font()));
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
    static constexpr Vector2 TEXT_POSITION{0, 25};
    static constexpr uint16_t TEXT_SIZE = 2;
    static constexpr int16_t SPACE_BETWEEN_BORDER = 10;

    TextBox text_box = make_text_box(TEXT_POSITION.x, TEXT_POSITION.y, PAGE_WIDTH, PAGE_HEIGHT, TEXT_SIZE);

    _display->drawLine(0, TEXT_POSITION.y - SPACE_BETWEEN_BORDER, PAGE_WIDTH, TEXT_POSITION.y - SPACE_BETWEEN_BORDER, 0);
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
    static constexpr Vector2 NUMBER_POSITION{0, 80};
    static constexpr Vector2 TITLE_POSITION{0, 240};
    static constexpr uint16_t NUMBER_TEXT_SIZE = 12;
    static constexpr uint16_t TITLE_TEXT_SIZE = 3;
    
    const std::string chapter_num_str = std::to_string(_current_chapter.num);
    const std::string chapter_title = _page_manager.get_chapter_title(_current_chapter.num);

    TextBox number_tb = make_text_box(NUMBER_POSITION.x, NUMBER_POSITION.y, PAGE_WIDTH, PAGE_HEIGHT, NUMBER_TEXT_SIZE);    
    TextBox title_tb = make_text_box(TITLE_POSITION.x, TITLE_POSITION.y, PAGE_WIDTH, PAGE_HEIGHT, TITLE_TEXT_SIZE);    

    number_tb.print_hebrew(TextHelper::serialize_to_font_indices(chapter_num_str, get_font()), CENETER_TEXT);
    title_tb.print_hebrew(TextHelper::serialize_to_font_indices(chapter_title, get_font()), CENETER_TEXT);
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

TextBox Book::make_text_box(int16_t left, int16_t top, int16_t right, int16_t bottom, uint16_t text_size)
{
    TextBox tb(
        _display,
        left,
        top,
        right, 
        bottom, 
        text_size,
        WritingDirection::RTL
    );
    tb.setFont(get_font());

    return tb;
}

const GFXfont* Book::get_font() const
{
    return &hebEng5x7avia;
}
