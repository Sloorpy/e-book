#include "Book.hpp"
#include "HebrewHelper.hpp"
#include <esp_log.h>

static constexpr char TAG[] = "Book";

Book::Book(const std::string_view& book_name, std::shared_ptr<SDManager> sd_reader, const Display& display) : 
    _page_manager(book_name, std::move(sd_reader)), 
    _chapters(),
    _display(display),
    _state{1,0} {}

std::string Book::get_page()
{
    Chapter& current_chapter = _chapters.at(_state.chapter - 1); 
    const std::vector<FontIndex> chars = HebrewHelper::process(reinterpret_cast<char*>(current_chapter.pages.data()));

    size_t i;
    for (i = current_chapter.pages_offset; i < chars.size(); ++i)
    {
        const FontIndex& ch = chars.at(i);
        CursorCalculation cursor = HebrewHelper::calculateCursor(_display, ch.index, ch.is_rtl);
        if (cursor.next_y > PAGE_HEIGHT)
        {
            break;
        }
    }

    current_chapter.pages_offset = i;
    return std::string(chars.begin() + current_chapter.pages_offset, chars.begin() + current_chapter.pages_offset + i);
}

void Book::next_page()
{   
    if (_state.chapter > _chapters.size())
    {
        _chapters.emplace_back(_page_manager.load_chapter(_state.chapter));
    }

    Chapter& current_chapter = _chapters.at(_state.chapter - 1); 

    const uint32_t page_size = current_chapter.pages.size();
    if (_state.page_offset >= page_size)
    {
        ++_state.chapter;
        _state.page_offset = 0;
        // (HERE): if we want to inforce buffering, we delete prev chapter (if there passed limit)
        _chapters.emplace_back(_page_manager.load_chapter(_state.chapter));
    }
}

void Book::prev_page()
{
    if (_state.chapter == 1 && _state.page_offset)
    {
        
    }
}

std::string Book::get_title()
{
    std::string cover = _page_manager.get_cover();
    size_t index = cover.find('\n');

    if (index == std::string::npos)
    {
        return cover;
    }

    return cover.substr(0, index);
}

std::string Book::get_author()
{
    std::string cover = _page_manager.get_cover();
    size_t index = cover.find('\n');

    if (index == std::string::npos)
    {
        return cover;
    }

    return cover.substr(index + 1);
}

bool Book::has_next_page() const
{
    if (_state.chapter > _page_manager.chapter_count())
    {
        return false;
    }
    

    return false;
}

bool Book::has_prev_page() const
{
    return false;
}
