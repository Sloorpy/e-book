#include "ProgramState.hpp"

ProgramState::ProgramState(std::shared_ptr<Display> display) :
    _display(display) {}

std::shared_ptr<Display> ProgramState::get_display()
{
    return _display;
}
