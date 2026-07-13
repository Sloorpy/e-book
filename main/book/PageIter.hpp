#pragma once
#include "PageBuffer.hpp"
#include "ui/Text/Layout/TextToken.hpp"

#include <memory>

struct PageRange final {
    size_t start;
    size_t end;
};

class PageIter final {
public:
    explicit PageIter(std::unique_ptr<PageBuffer> buffer);
    ~PageIter() = default;

public: 
    bool has_next() const;
    bool has_prev() const;

public:
    TextPage next();
    TextPage prev();

private:
    static std::vector<PageRange> initialize_page_indecies(PageBuffer& buffer);

private:
    size_t _current_page;
    std::vector<PageRange> _page_indecies;
    const std::unique_ptr<PageBuffer> _buffer;
};