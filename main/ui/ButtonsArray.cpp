#include "ButtonsArray.hpp"
#include <stdexcept>
#include <utility>
ButtonsArray::ButtonsArray(std::array<UIButton, ARRAY_SIZE> buttons) :
    _array(std::move(buttons)),
    _current_index(0)
{}

const UIButton &ButtonsArray::next()
{
    _current_index = next_index();
    return head();
}

const UIButton &ButtonsArray::prev()
{
    _current_index = prev_index();
    return head();

}

const UIButton &ButtonsArray::head()
{
    if (_current_index >= ARRAY_SIZE) {
        throw std::runtime_error("Selected button index is out of range");
    }

    return _array[_current_index];
}

const UIButton &ButtonsArray::peak_next()
{
    return _array[next_index()];
}

const UIButton &ButtonsArray::peak_prev()
{
    return _array[prev_index()];
}

size_t ButtonsArray::prev_index()
{
    if (_current_index == 0) {
        return ARRAY_SIZE - 1;
    }

    return (_current_index - 1) % ARRAY_SIZE;
}

size_t ButtonsArray::next_index()
{
    return (_current_index + 1) % ARRAY_SIZE;
}
