#pragma once

#include "UIComponent.hpp"
#include "Geometry.hpp"

#include <gfxfont.h>
#include <string>

class TextBox;

class HomeButton final : public UIComponent
{
public:
    struct Layout {
        Rect bounds;
        const GFXfont* font;
        int16_t radius;
    };

    HomeButton(std::string label, Layout layout);
    void draw(CanvasRegion& region) override;

    void hover()    { _hovered = true; }
    void unhover()  { _hovered = false; }
    void select()   { _selected = true; }
    void deselect() { _selected = false; }
    bool is_selected() { return _selected; } 
private:
    void draw_hover(CanvasRegion& region);

private:
    TextBox create_label_box(CanvasRegion& region) const;

private:
    std::string _label;
    Layout _layout;
    bool _hovered = false;
    bool _selected = false;
};
