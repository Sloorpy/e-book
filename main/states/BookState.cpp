#include "BookState.hpp"
#include "Text/TextBox.hpp"
#include "Fonts/hebEng5x7avia.h"

#include <esp_timer.h>

BookState::BookState(const std::string_view& book_name, std::unique_ptr<ProgramState> prev_state) :
    ProgramState(prev_state->get_display()),
    _book(create_book(book_name))
{}


BookState::BookState(const std::string_view& book_name, std::unique_ptr<Display> display) :
    ProgramState(std::move(display)),
    _book(create_book(book_name))
{}

void BookState::main()
{
    _display->fill_screen(Color::WHITE);

    _book->display_title(); // Show text

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
    _display->fill_screen(Color::WHITE);

    if (!_book->has_prev_page()) {
        _book->display_title();
    }
    else {
        _book->prev_page();
    }
    _display->update();
}

void BookState::on_hold()
{
    _book->reset_book();
    main();
}

std::unique_ptr<Book> BookState::create_book(const std::string_view &book_name)
{
    return std::make_unique<Book>(_display, book_name);
}
