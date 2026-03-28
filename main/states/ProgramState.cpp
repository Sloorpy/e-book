#include "ProgramState.hpp"

ProgramState::ProgramState(std::unique_ptr<Display> display) :
    _display(std::move(display)) {}

std::unique_ptr<Display> ProgramState::give_up_display()
{
    return std::move(_display);
}
