#pragma once

#include "Display.hpp"

class ScopedDisplayFont final {
public:
    ScopedDisplayFont(Display& display, const GFXfont* new_font);
    ~ScopedDisplayFont() noexcept;

public:
    ScopedDisplayFont(const ScopedDisplayFont&) = delete;
    ScopedDisplayFont& operator=(const ScopedDisplayFont&) = delete;
    ScopedDisplayFont(ScopedDisplayFont&&) = delete;
    ScopedDisplayFont& operator=(ScopedDisplayFont&&) = delete;

private:
    Display& _display;
    const GFXfont* _old_font;
};
