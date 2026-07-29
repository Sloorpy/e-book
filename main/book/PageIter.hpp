#pragma once
#include "PageBuffer.hpp"
#include "Text/Rendering/TextBox.hpp"
#include "ui/Text/Layout/TextToken.hpp"

#include <memory>

class PageIter final {
public:
    explicit PageIter(std::unique_ptr<PageBuffer> buffer, std::unique_ptr<TextBox> tb_mock, const Direction direction);
    ~PageIter() = default;

public: 
    bool has_next() const;
    bool has_prev() const;

public:
    TextPage next();
    TextPage prev();

private:
    static std::vector<PageRange> initialize_page_indecies(PageBuffer& buffer, TextBox& tb_mock, const Direction direction);

private:
    size_t _current_page;
    std::vector<PageRange> _page_indecies;
    const std::unique_ptr<PageBuffer> _buffer;
    const std::unique_ptr<TextBox> _tb_mock;
    const Direction _direction;
};
