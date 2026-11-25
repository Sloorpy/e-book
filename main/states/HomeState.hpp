#pragma once
#include "IState.hpp"
#include "ButtonsArray.hpp"

class HomeState final : public IState {
public:
    explicit HomeState(std::unique_ptr<IState> prev_state);
    explicit HomeState(std::unique_ptr<Display> display);
    ~HomeState() override = default;

public:
    virtual void main() override;
    virtual void on_right_click() override;
    virtual void on_left_click() override;
    virtual void on_middle_click() override;
private:
    explicit HomeState(std::shared_ptr<Display> display);

    CanvasRegion _button_row;
    ButtonsArray _buttons;
};
