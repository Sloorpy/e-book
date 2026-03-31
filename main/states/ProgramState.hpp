#pragma once
#include "Book.hpp"
#include "Button.hpp"
#include "Display.hpp"

class ProgramState 
{
protected:
    explicit ProgramState(std::shared_ptr<Display> display);

public:
    virtual ~ProgramState() = default;

public:
    virtual void main() = 0;
    virtual void on_click() = 0;
    virtual void on_double_click() = 0;
    virtual void on_hold() = 0;

public:
    std::shared_ptr<Display> get_display();
    
protected:
    std::shared_ptr<Display> _display;
};