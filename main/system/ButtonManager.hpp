#pragma once
#include "Button.hpp"

#include <memory>
#include <vector>

struct Click {
    uint64_t _click_time;
    static constexpr uint64_t BUTTON_READY_TIME_MS = 200;
};

enum class ButtonType : uint8_t{
    LEFT = 0,
    RIGHT,
    MIDDLE
};

class ButtonManager final {
public:
    explicit ButtonManager();

public:
    void add_button(std::unique_ptr<Button> button);

private:
    std::vector<std::unique_ptr<Button>> _buttons;
    static constexpr uint64_t DEBOUNCE_MS = 35;
    static constexpr uint64_t BUTTON_POLL_INTERVAL_MS = 15;
};