#pragma once

class CanvasRegion;

class UIComponent
{
public:
    virtual ~UIComponent() = default;
    virtual void draw(CanvasRegion& region) = 0;
};
