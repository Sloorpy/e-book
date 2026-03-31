#include "BookState.hpp"
#include "TextBox.hpp"
#include "Fonts/hebEng5x7avia.h"

BookState::BookState(const std::string_view& book_name, std::shared_ptr<SDManager> sd, std::unique_ptr<ProgramState> prev_state) :
    ProgramState(prev_state->get_display()),
    _book(create_book(book_name, sd))
{}


BookState::BookState(const std::string_view& book_name, std::shared_ptr<SDManager> sd, std::unique_ptr<Display> display) :
    ProgramState(std::move(display)),
    _book(create_book(book_name, sd))
{}

void BookState::main()
{
    _display->fill_screen(Color::WHITE);

    _book->display_title(); // Show text
    _display->drawRect(90, 180, 120, 180, 0); // Show book image 

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
