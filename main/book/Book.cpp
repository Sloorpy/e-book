#include "Book.hpp"
#include "Text/Support/TextHelper.hpp"
#include "Text/Rendering/TextBox.hpp"
#include "Fonts/hebEng5x7avia.h"
#include "Fonts/FreeMonoBold9pt7b.h"
#include "Display.hpp"

#include <string_view>
#include <esp_log.h>
#include <esp_timer.h>
#include <cstring>
#include <algorithm>

static constexpr char TAG[] = "Book";
static const Chapter BASE_CHAPTER{{}, {}, "", 0};

static constexpr Vector2 TEXT_POSITION{0, 18};
static constexpr uint16_t TEXT_SIZE = 2;

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
    switch(_view_state) {
        case BookViewState::BOOK_TITLE:
            display_title();
            break;
        case BookViewState::CHAPTER_TITLE:
            display_chapter_title();
            break;
        case BookViewState::FINISHED:
            no_more_pages();
            break;
        case BookViewState::PAGE:
            display_header();    
            display_current_page();
            break;
    }
}

void Book::render_and_save()
{
    render_current_view();
    _page_manager.save_state(current_state());
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

    title_tb.print(TextHelper::serialize_to_font_indices(title, get_font()), InitialPosition::Center);
    author_tb.print(TextHelper::serialize_to_font_indices(author, get_font()));
    
    static const Vector2 bitmap_position{(PAGE_WIDTH - static_cast<int16_t>(BITMAP_WIDTH)) / 2, 150};
    draw_cover(bitmap_position.x, bitmap_position.y);
}

void Book::no_more_pages()
{
    static constexpr Vector2 TEXT_POSITION{0, 50};
    static constexpr uint16_t TEXT_SIZE = 3;

    TextBox text_box = make_text_box(TEXT_POSITION.x, TEXT_POSITION.y, PAGE_WIDTH, PAGE_HEIGHT, TEXT_SIZE);
    
    static constexpr std::string_view msg = "נגמרו העמודים :)";
    text_box.print(TextHelper::serialize_to_font_indices(std::string(msg), get_font()));
}

size_t Book::print_page_size(const std::vector<uint8_t>& text)
{
    return make_page_text_box().next_print_size(text);
}

void Book::display_current_page()
{
    if (_current_chapter.page_indicies.empty()) {
        throw std::runtime_error("No current page found. Can't load page");
    }

    const std::vector<uint8_t> curr_text(
        _current_chapter.pages.begin() + _current_chapter.page_indicies.top().start,
        _current_chapter.pages.begin() + _current_chapter.page_indicies.top().end
    );

    make_page_text_box().print(curr_text, InitialPosition::Right);
}

void Book::reset_book()
{
    _current_chapter = BASE_CHAPTER;
    _view_state = BookViewState::BOOK_TITLE;
    render_current_view();
}

void Book::first_chapter_title()
{
    if (_current_chapter.num != 1) {
        _current_chapter = _page_manager.load_chapter(1, get_font());
    }

    _view_state = BookViewState::CHAPTER_TITLE;
}

void Book::load_all_pages()
{
    if (_current_chapter.page_indicies.empty() && _current_chapter.num > 0) {
        build_page_indices(_current_chapter.pages.size());
    }

    if (!_current_chapter.page_indicies.empty()) {
        _view_state = BookViewState::PAGE;
    }
    else {
        _view_state = BookViewState::CHAPTER_TITLE;
    }
}

void Book::prev_chapter_last_page()
{
    if (_current_chapter.num <= 1) {
        _view_state = BookViewState::BOOK_TITLE;
        _current_chapter = BASE_CHAPTER;
        return;
    }

    const uint16_t prev_chapter = _current_chapter.num - 1;
    _current_chapter = _page_manager.load_chapter(prev_chapter, get_font());
    build_page_indices(_current_chapter.pages.size());

    if (!_current_chapter.page_indicies.empty()) {
        _view_state = BookViewState::PAGE;
    }
    else {
        _view_state = BookViewState::CHAPTER_TITLE;
    }
}

void Book::next_page()
{
    switch (_view_state) {
        case BookViewState::BOOK_TITLE:
            first_chapter_title();
            render_and_save();
            break;

        case BookViewState::FINISHED:
            no_more_pages();
            break;

        case BookViewState::CHAPTER_TITLE:
            _view_state = BookViewState::PAGE;
            [[fallthrough]];

        case BookViewState::PAGE:
            _view_state = handle_next_chapter();
            render_and_save();
            break;
    }
}

void Book::prev_page()
{
    switch (_view_state) {
        case BookViewState::BOOK_TITLE:
            render_current_view();
            break;

        case BookViewState::CHAPTER_TITLE:
            prev_chapter_last_page();
            render_and_save();
            break;

        case BookViewState::FINISHED:
            load_all_pages();
            render_and_save();
            break;

        case BookViewState::PAGE:
            _view_state = handle_prev_chapter();
            render_and_save();
            break;
    }
}

void Book::curr_page()
{
    render_and_save();
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

    number_tb.print(TextHelper::serialize_to_font_indices(chapter_num_str, get_font()), InitialPosition::Center);
    title_tb.print(TextHelper::serialize_to_font_indices(chapter_title, get_font()), InitialPosition::Center);
}

void Book::display_header()
{
    _display->drawLine(0, TEXT_POSITION.y, PAGE_WIDTH, TEXT_POSITION.y, 0);
    const uint16_t part_width =  _display->width() / 5;
    TextBox left_tb = make_header_text_box(0, part_width * 2);
    TextBox middle_tb = make_header_text_box(part_width * 2, TEXT_POSITION.x + part_width * 3);
    TextBox right_tb = make_header_text_box(part_width * 3, TEXT_POSITION.x + part_width * 5);
    
    const uint32_t pages_read = (_current_chapter.num - 1) * _current_chapter.pages.size() +
                     (_current_chapter.page_indicies.empty() ? 0 : _current_chapter.page_indicies.top().end);
    const uint32_t finished_percentage = (pages_read * 100) / (_current_chapter.pages.size() * _page_manager.chapter_count());
    printf("pages_read %ld\nall pages %d\nfinished_percentage %ld\n", pages_read,_current_chapter.pages.size() * _page_manager.chapter_count(), finished_percentage);
    
    left_tb.print(TextHelper::serialize_to_font_indices("Chapter: " + std::to_string(_current_chapter.num), left_tb.font()), InitialPosition::Left);
    middle_tb.print(TextHelper::serialize_to_font_indices("|", middle_tb.font()), InitialPosition::Center);
    right_tb.print(TextHelper::serialize_to_font_indices( std::to_string(finished_percentage) + "%", right_tb.font()), InitialPosition::Right);
}

void Book::draw_cover(const uint16_t start_x, const uint16_t start_y)
{
    std::vector<uint8_t> bitmap = _page_manager.cover_bitmap();
    if (bitmap.empty()) {
        return;
    }
    
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
    const size_t chapter_offset =  std::min(state.index, _current_chapter.pages.size());
    
    if (_current_chapter.num >= _page_manager.chapter_count() && chapter_offset >= _current_chapter.pages.size()) {
        _view_state = BookViewState::FINISHED;
    } 
    else if (chapter_offset > 0) {
        _view_state = BookViewState::PAGE;
    }
    else {
        _view_state =  BookViewState::CHAPTER_TITLE;
    }

    build_page_indices(chapter_offset);
}

TextBox Book::make_header_text_box(const int16_t start_x, const int16_t end_x) const
{
    TextBox tb(
        _display,
        start_x,
        0,
        end_x,
        TEXT_POSITION.y,
        get_font(),
        2
    );

    return tb;
}

TextBox Book::make_page_text_box() const
{   
    static constexpr int16_t SPACE_FROM_BORDER = 7;
     TextBox tb(
        _display,
        0,
        TEXT_POSITION.y + SPACE_FROM_BORDER,
        PAGE_WIDTH, 
        PAGE_HEIGHT, 
        get_font(),
        TEXT_SIZE
    );

    return tb;
}

TextBox Book::make_text_box(int16_t left, int16_t top, int16_t right, int16_t bottom, uint16_t text_size) const
{
     TextBox tb(
        _display,
        left,
        top,
        right, 
        bottom,                 
        get_font(),
        text_size
    );

    return tb;
}

StateInfo Book::current_state() const
{
    if (_view_state == BookViewState::BOOK_TITLE) {
        return StateInfo{0, 0};
    }

    if (_view_state == BookViewState::CHAPTER_TITLE || _current_chapter.page_indicies.empty()) {
        return StateInfo{_current_chapter.num, 0};
    }

    return StateInfo{_current_chapter.num, _current_chapter.page_indicies.top().end};
}

const GFXfont* Book::get_font() const
{
    return &hebEng5x7avia;
}

BookViewState Book::handle_next_chapter()
{
    if (!_current_chapter.page_indicies.empty() && _current_chapter.page_indicies.top().end >= _current_chapter.pages.size())
    {
        const uint16_t next_chapter = _current_chapter.num + 1;
        if (next_chapter > _page_manager.chapter_count()) {
            return BookViewState::FINISHED;
        }

        _current_chapter = _page_manager.load_chapter(next_chapter, get_font());
        return BookViewState::CHAPTER_TITLE;
    }
    
    size_t page_start = 0;
    if (!_current_chapter.page_indicies.empty()) {
        page_start = _current_chapter.page_indicies.top().end;
    }
    
    const std::vector<uint8_t> curr_text(
        _current_chapter.pages.begin() + page_start,
        _current_chapter.pages.end()
    );

    const size_t bytes_written = print_page_size(curr_text);   
    if (bytes_written == 0) {
        throw std::runtime_error("Tried to get print page size for `next_page` but got size of 0");
    }

    _current_chapter.page_indicies.push(PageRange{page_start, page_start + bytes_written});

    return BookViewState::PAGE;
}

BookViewState Book::handle_prev_chapter()
{
    if (_current_chapter.page_indicies.empty()) {
        return BookViewState::CHAPTER_TITLE;
    }

    _current_chapter.page_indicies.pop();

    if (_current_chapter.page_indicies.empty()) {
        return BookViewState::CHAPTER_TITLE;
    }

    return BookViewState::PAGE;
}

void Book::build_page_indices(size_t end_offset)
{
    if (end_offset >= _current_chapter.pages.size()) {
        end_offset = _current_chapter.pages.size();
    }

    _current_chapter.page_indicies = std::stack<PageRange>();
 
    size_t offset = 0;
    TextBox temp_tb = make_page_text_box();
    while (offset < end_offset) {
        const size_t start_offset = offset;

        const std::vector<uint8_t> page(
            _current_chapter.pages.begin() + offset,
            _current_chapter.pages.end()
        );

        const size_t page_size = temp_tb.next_print_size(page);
        if (page_size == 0) {
            break;
        }

        offset += page_size;
        _current_chapter.page_indicies.push(PageRange{start_offset, offset});
    }
}
