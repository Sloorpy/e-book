#include "HomeState.hpp"
#include "Config.hpp"

#include <utility>

HomeState::HomeState(std::unique_ptr<IState> prev_state) :
    HomeState(prev_state->get_display())
{}

HomeState::HomeState(std::unique_ptr<Display> display) :
    HomeState(std::shared_ptr<Display>(std::move(display)))
{}

HomeState::HomeState(std::shared_ptr<Display> display) :
    IState(std::move(display)),
    _button_row(_display->create_canvas({0, static_cast<uint16_t>(_display->height() - Config::Home::ROW_HEIGHT),
                                         Config::Display::WIDTH, Config::Home::ROW_HEIGHT}))
{}

void HomeState::main()
{
    _buttons.draw(_button_row);
    _display->update();
}

void HomeState::on_right_click()
{
    if (_buttons.next()) {
        _buttons.draw(_button_row);
        _display->update();
    }
}

void HomeState::on_left_click()
{
    if (_buttons.prev()) {
        _buttons.draw(_button_row);
        _display->update();
    }
}

void HomeState::on_middle_click()
{
    if (_buttons.select()) {
        _buttons.draw(_button_row);
        _display->update();
    }
}

