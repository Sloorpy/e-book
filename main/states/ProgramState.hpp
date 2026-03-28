#pragma once
#include "Book.hpp"
#include "Display.hpp"
#include "Button.hpp"

class ProgramState 
{
protected:
    explicit ProgramState(std::unique_ptr<Display> display);

public:
    virtual ~ProgramState() = default;

public:
    virtual void main() = 0;
    virtual void on_click() = 0;
    virtual void on_double_click() = 0;
    virtual void on_hold() = 0;

public:
    std::unique_ptr<Display> give_up_display();
    
protected:
    std::unique_ptr<Display> _display;
};