#pragma once
#include "core/UIButton.hpp"

#include <array>
#include <cstddef>

class ButtonsArray final {
private:
    static constexpr size_t ARRAY_SIZE = 3;
public:
    explicit ButtonsArray( std::array<UIButton, ARRAY_SIZE> buttons);

public:
    const std::array<UIButton, ARRAY_SIZE>& buttons() const { return _array; }

    const UIButton& next();
    const UIButton& prev();
    const UIButton& head();
    const UIButton& peak_next();
    const UIButton& peak_prev();

private: 
    size_t prev_index();
    size_t next_index();
    
private:
    std::array<UIButton, ARRAY_SIZE> _array;
    size_t _current_index;
};
