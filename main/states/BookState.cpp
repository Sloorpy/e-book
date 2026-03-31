#include "BookState.hpp"
#include "TextBox.hpp"
#include "Fonts/hebEng5x7avia.h"

BookState::BookState(const std::string_view& book_name, std::shared_ptr<SDManager> sd, std::unique_ptr<ProgramState> prev_state) :
    ProgramState(prev_state->get_display()),
    _book(create_book(book_name, sd)),
    _sd(sd)
{}


BookState::BookState(const std::string_view& book_name, std::shared_ptr<SDManager> sd, std::unique_ptr<Display> display) :
    ProgramState(std::move(display)),
    _book(create_book(book_name, sd)),
    _sd(sd)
{}

void BookState::main()
{
    _display->fill_screen(Color::WHITE);

    _book->display_title(); // Show text
    
    static constexpr uint16_t BITMAP_WIDTH = 160;
    static constexpr uint16_t BITMAP_HEIGHT = 210;
    static constexpr uint16_t BITMAP_Y = 150;
    static uint16_t BITMAP_X = (_display->width() - static_cast<int16_t>(BITMAP_WIDTH)) / 2;
    
    std::vector<uint8_t> bitmap = File(_sd, "books/percy_2_heb/cover.bin").read_all_bytes();
    _display->drawRect(BITMAP_X, BITMAP_Y, BITMAP_WIDTH, BITMAP_HEIGHT, 0); // Show book image 
    _display->drawBitmap(BITMAP_X, BITMAP_Y, bitmap.data(), BITMAP_WIDTH, BITMAP_HEIGHT, static_cast<uint16_t>(Color::BLACK));
    _display->update();
}

void BookState::on_click()
{
    _display->fill_screen(Color::WHITE);

    if (!_book->has_next_page()) {
        _book->no_more_pages();
    }
    else {
        _book->read_page();
    }
    _display->update();
}

void BookState::on_double_click()
{
}

void BookState::on_hold()
{
}

std::unique_ptr<Book> BookState::create_book(const std::string_view &book_name, std::shared_ptr<SDManager> sd)
{
    return std::make_unique<Book>(_display, book_name, sd);
}
