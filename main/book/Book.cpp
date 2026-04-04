#include "Book.hpp"
#include "Text/TextHelper.hpp"
#include "Text/TextBox.hpp"
#include "Fonts/hebEng5x7avia.h"
#include "Display.hpp"

#include <string_view>
#include <esp_log.h>
#include <esp_timer.h>
#include <cstring>
#include <algorithm>

static constexpr char TAG[] = "Book";
static constexpr bool CENETER_TEXT = true;
static const Chapter BASE_CHAPTER{{}, {}, "", 0, 0};

static constexpr Vector2 TEXT_POSITION{0, 26};
static constexpr uint16_t TEXT_SIZE = 2;
static constexpr int16_t SPACE_BETWEEN_BORDER = 10;
static constexpr uint16_t BORDER_Y = TEXT_POSITION.y - SPACE_BETWEEN_BORDER;

Book::Book(std::shared_ptr<Display> display, const std::string_view& book_name) : 
    _page_manager(book_name),
    _current_chapter(BASE_CHAPTER),
    _view_state(BookViewState::BOOK_TITLE),
    _display(display) 
{
    load_chapter_save();
}

void Book::render_current_view()
{
    if (_view_state == BookViewState::BOOK_TITLE) {
        display_title();
        return;
    }
    
    if (_view_state == BookViewState::CHAPTER_TITLE) {
        handle_chapter_title();
        return;
    }

    if (_view_state == BookViewState::FINISHED) {
        no_more_pages();
        return;
    }
    
    print_page();
    display_header();
}

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
    
    static constexpr std::string_view msg = "נגמרו העמודים :)";
    text_box.print_hebrew(TextHelper::serialize_to_font_indices(std::string(msg), get_font()));
}

size_t Book::print_page()
{
    const std::vector<uint8_t> curr_text(
        _current_chapter.pages.begin() + _current_chapter.pages_offset,
        _current_chapter.pages.end()
    );

    return make_page_text_box().print_hebrew(curr_text);
}

void Book::reset_book()
{
    _current_chapter = BASE_CHAPTER;
    _view_state = BookViewState::BOOK_TITLE;
    render_current_view();
}

void Book::next_page()
{
    switch(_view_state) {
        case BookViewState::BOOK_TITLE:
            if (_current_chapter.num != 1) {
                _current_chapter = _page_manager.load_chapter(1, get_font());
            }

            _view_state = BookViewState::CHAPTER_TITLE;
            _page_manager.save_state(current_state());
            render_current_view();
            break;
        case BookViewState::FINISHED:
            no_more_pages();
            break;
        case BookViewState::CHAPTER_TITLE:
            _view_state = BookViewState::PAGE;
            [[fallthrough]];
        case BookViewState::PAGE:
            read_page();
            _page_manager.save_state(current_state());
            break;
    }
}

void Book::prev_page()
{
    if (_view_state == BookViewState::BOOK_TITLE) {
        render_current_view();
        return;
    }

    if (_view_state == BookViewState::FINISHED) {
        if (_current_chapter.page_indicies.empty() && _current_chapter.num > 0) {
            build_page_indices(_current_chapter.pages.size());
        }

        _view_state = handle_prev_chapter();
        render_current_view();
        _page_manager.save_state(current_state());
        return;
    }

    if (_view_state == BookViewState::CHAPTER_TITLE) {
        if (_current_chapter.num <= 1) {
            _view_state = BookViewState::BOOK_TITLE;
            render_current_view();
            _page_manager.save_state(current_state());
            return;
        }

        // go back a chapter
        const uint16_t prev_chapter = _current_chapter.num - 1;
        _current_chapter = _page_manager.load_chapter(prev_chapter, get_font());
        build_page_indices(_current_chapter.pages.size());
    }

    _view_state = handle_prev_chapter();
    render_current_view();
    _page_manager.save_state(current_state());
}

void Book::read_page()
{
    if (_view_state != BookViewState::PAGE) {
        return;
    }

    _current_chapter.page_indicies.push(_current_chapter.pages_offset);

    const size_t bytes_written = print_page();
    display_header();

    _current_chapter.pages_offset += bytes_written;

    _view_state = handle_next_chapter();
}

void Book::handle_chapter_title()
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

void Book::display_header()
{
    _display->drawLine(0, BORDER_Y, PAGE_WIDTH, BORDER_Y, 0);
}

size_t Book::current_page_size() const
{
    const std::vector<uint8_t> curr_text(
        _current_chapter.pages.begin() + _current_chapter.pages_offset,
        _current_chapter.pages.end()
    );

    return make_page_text_box().next_print_size(curr_text);
}

void Book::draw_cover(const uint16_t start_x, const uint16_t start_y)
{
    std::vector<uint8_t> bitmap = File("books/percy_2_heb/cover.bin", "rb").read_all_bytes();
    _display->drawRect(start_x, start_y, BITMAP_WIDTH, BITMAP_HEIGHT, 0);
    _display->drawBitmap(start_x, start_y, bitmap.data(), BITMAP_WIDTH, BITMAP_HEIGHT, static_cast<uint16_t>(Color::BLACK));
}

bool Book::has_next_page() const
{
    return _view_state != BookViewState::FINISHED;
}

bool Book::has_prev_page() const
{
    if (_view_state == BookViewState::BOOK_TITLE) {
        return false;
    }

    if (_view_state == BookViewState::FINISHED) {
        return !_current_chapter.page_indicies.empty() || _current_chapter.num > 0;
    }

    if (_view_state == BookViewState::CHAPTER_TITLE) {
        return _current_chapter.num > 1;
    }

    return !_current_chapter.page_indicies.empty();
}

void Book::load_chapter_save()
{
    StateInfo state = _page_manager.load_state();

    uint16_t chapter_num = state.chapter_num;
    if (chapter_num <= 0 || chapter_num > _page_manager.chapter_count()) {
        return;
    }

    _current_chapter = _page_manager.load_chapter(chapter_num, get_font());
    _current_chapter.pages_offset = std::min(state.index, _current_chapter.pages.size());
    _view_state = _current_chapter.pages_offset > 0 ? BookViewState::PAGE : BookViewState::CHAPTER_TITLE;

    build_page_indices(_current_chapter.pages_offset);
}

TextBox Book::make_header_text_box() const
{
    return make_text_box(TEXT_POSITION.x, TEXT_POSITION.y, PAGE_WIDTH, PAGE_HEIGHT, TEXT_SIZE);
}

TextBox Book::make_page_text_box() const
{
    return make_text_box(TEXT_POSITION.x, TEXT_POSITION.y, PAGE_WIDTH, PAGE_HEIGHT, TEXT_SIZE);
}

StateInfo Book::current_state() const
{
    if (_view_state == BookViewState::BOOK_TITLE) {
        return StateInfo{0, 0};
    }

    if (_view_state == BookViewState::FINISHED) {
        return StateInfo{0, 0};
    }

    if (_view_state == BookViewState::CHAPTER_TITLE) {
        return StateInfo{_current_chapter.num, 0};
    }

    return StateInfo{_current_chapter.num, _current_chapter.pages_offset};
}

TextBox Book::make_text_box(int16_t left, int16_t top, int16_t right, int16_t bottom, uint16_t text_size) const
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

BookViewState Book::handle_next_chapter()
{
    if (_current_chapter.pages_offset < _current_chapter.pages.size()) {
        return BookViewState::PAGE;
    }

    const uint16_t next_chapter = _current_chapter.num + 1;
    if (next_chapter <= _page_manager.chapter_count()) {
        _current_chapter = _page_manager.load_chapter(next_chapter, get_font());
        return BookViewState::CHAPTER_TITLE;
    }

    return BookViewState::FINISHED;
}

BookViewState Book::handle_prev_chapter()
{
    return step_back_within_loaded_chapter();
}

BookViewState Book::step_back_within_loaded_chapter()
{
    if (_current_chapter.page_indicies.empty()) {
        _current_chapter.pages_offset = 0;
        return BookViewState::CHAPTER_TITLE;
    }

    _current_chapter.page_indicies.pop();

    if (_current_chapter.page_indicies.empty()) {
        _current_chapter.pages_offset = 0;
        return BookViewState::CHAPTER_TITLE;
    }

    _current_chapter.pages_offset = _current_chapter.page_indicies.top();
    return BookViewState::PAGE;
}

void Book::build_page_indices(size_t end_offset)
{
    if (end_offset >= _current_chapter.pages.size()) {
        end_offset = _current_chapter.pages.size();
    }

    _current_chapter.page_indicies = std::stack<size_t>();

    size_t offset = 0;
    TextBox temp_tb = make_page_text_box();
    while (offset < end_offset) {
        _current_chapter.page_indicies.push(offset);

        const std::vector<uint8_t> page(
            _current_chapter.pages.begin() + offset,
            _current_chapter.pages.end()
        );

        const size_t page_size = temp_tb.next_print_size(page);
        if (page_size == 0) {
            break;
        }
        
        offset += page_size;
    }
}