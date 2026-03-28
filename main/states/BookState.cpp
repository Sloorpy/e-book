#include "BookState.hpp"

BookState::BookState(const std::string_view& book_name, std::shared_ptr<SDManager> sd, std::unique_ptr<ProgramState> prev_state) :
    ProgramState(prev_state->give_up_display()),
    _book(std::make_unique<Book>(book_name, sd, *_display))
{
}


BookState::BookState(const std::string_view& book_name, std::shared_ptr<SDManager> sd, std::unique_ptr<Display> display) :
    ProgramState(std::move(display)),
    _book(std::make_unique<Book>(book_name, sd, *_display))
{
}

void BookState::main()
{
    _display->fill_screen(Color::WHITE);
    display_title();

    _display->update();
}

void BookState::on_click()
{
    _display->fill_screen(Color::WHITE);

    if (!_book->has_next_page()) {
        no_more_pages();
        _display->update();
        return;
    }
}

void BookState::on_double_click()
{
}

void BookState::on_hold()
{
}

void BookState::display_title() const
{
    static constexpr uint16_t MIDDLE_X = 270;
    static constexpr uint16_t MIDDLE_Y = 75;
    _display->setCursor(MIDDLE_X, MIDDLE_Y);

    _display->setTextSize(5);
    _display->print_hebrew(_book->get_title().c_str());


    static constexpr uint16_t BOTTOM_X = 300;
    static constexpr uint16_t BOTTOM_Y = 375;
    _display->setCursor(BOTTOM_X, BOTTOM_Y);

    _display->setTextSize(2);
    _display->print_hebrew(_book->get_author().c_str());
    _display->drawRect(90, 180, 120, 180, 0);
}

void BookState::no_more_pages() const
{
    _display->setTextSize(3);
    _display->setCursor(300, 100);
    _display->print_hebrew("נגמרו העמודים :)");
}
