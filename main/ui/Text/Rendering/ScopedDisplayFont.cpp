#include "ScopedDisplayFont.hpp"

ScopedDisplayFont::ScopedDisplayFont(Display& display, const GFXfont* new_font)
    : _display(display)
    , _old_font(display.getFont())
{
    _display.setFont(new_font);
}

ScopedDisplayFont::~ScopedDisplayFont() noexcept
{
    _display.setFont(_old_font);
}
