#pragma once
#include "Book.hpp"
#include "Button.hpp"
#include "Display.hpp"

class IState {
protected:
    explicit IState(std::shared_ptr<Display> display) 
        : _display(display) {}

public:
    virtual ~IState() = default;

public:
    virtual void main() = 0;
    virtual void on_right_click() = 0;
    virtual void on_left_click() = 0;
    virtual void on_middle_click() = 0;

public:
    std::shared_ptr<Display> get_display() { return _display; }
    
protected:
    std::shared_ptr<Display> _display;
};