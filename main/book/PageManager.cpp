#include "PageManager.hpp"
#include "Text/TextHelper.hpp"
#include <esp_log.h>
#include <string>

static constexpr char TAG[] = "PageManager";


PageManager::PageManager(const std::string_view &book_name) :
    _book_name(book_name),
    _chapters_count(0)
{
    _chapters_count = chapter_count();
}


std::string PageManager::get_title() const
{
    const std::string data = File(book_file_path("cover")).read_all();
    size_t index = data.find('\n');

    if (index == std::string::npos)
    {
        return data;
    }

    return data.substr(0, index);
}

std::string PageManager::get_author() const
{
    const std::string data = File(book_file_path("cover")).read_all();
    size_t index = data.find('\n');

    if (index == std::string::npos)
    {
        return data;
    }

    return data.substr(index + 1);
}

std::string PageManager::get_chapter_title(const uint16_t chapter_num) const
{
    return File(chapter_file_path(chapter_num, "chapter")).read_all();
}

uint16_t PageManager::chapter_count() const
{
    if (_chapters_count > 0) {
        return _chapters_count;
    }

    const std::string chapter_count_path = book_file_path("chapter_count");
    const std::string chapter_count = File(chapter_count_path).read_all();

    int num = 0;
    try {
        num = std::stoi(chapter_count);
    }
    catch (...) {
        throw std::runtime_error("Failed to parse chapter count to int");
    }

    static constexpr int MAX_UINT16 = 65535;
    static constexpr int MIN_UINT16 = 0;

    if (num < MIN_UINT16 || num > MAX_UINT16) {
        throw std::runtime_error("Chapter count out of uint16_t range");
    }

    return static_cast<uint16_t>(num);
}

StateInfo PageManager::load_state(const uint16_t state_num)
{
    const std::string path = book_file_path("state_" + std::to_string(state_num));
    const std::string data = File(path).read_all();
    const size_t split_offset = data.find_first_of('\n'); 
    const std::string chapter_num_str = data.substr(0, split_offset);
    const std::string index_str = data.substr(split_offset + 1);
    uint16_t chapter_val = 0;
    size_t index = 0;
    std::from_chars(chapter_num_str.c_str(), chapter_num_str.c_str() + chapter_num_str.size(), chapter_val);
    std::from_chars(index_str.c_str(), index_str.c_str() + index_str.size(), index);

    return StateInfo{
        chapter_val,
        index
    };
}

std::string PageManager::book_root_path() const
{
    std::string path;
    path.reserve(6 + _book_name.size() + 1);
    path += "books/";
    path += _book_name;
    path += '/';
    return path;
}

std::string PageManager::book_file_path(const std::string_view& filename) const
{
    std::string path = book_root_path();
    path += filename;
    return path;
}

std::string PageManager::chapter_root_path(uint16_t chapter_num) const
{
    std::string path = book_root_path();
    path += std::to_string(chapter_num);
    path += '/';
    return path;
}

std::string PageManager::chapter_file_path(uint16_t chapter_num, const std::string_view& filename) const
{
    std::string path = chapter_root_path(chapter_num);
    path += filename;
    return path;
}

Chapter PageManager::load_chapter(const uint16_t chapter_num, const GFXfont* font)
{
    if (chapter_num > chapter_count())
    {
        throw std::runtime_error("Error: Tried to get invalid chapter");
    }

    const std::string chapter_path = chapter_file_path(chapter_num, "chapter");
    const std::string pages_path = chapter_file_path(chapter_num, "pages");

    static constexpr size_t START_OFFSET = 0;
    return Chapter{
        TextHelper::serialize_to_font_indices(File(pages_path).read_all_bytes(), font),
        File(chapter_path).read_all(),
        START_OFFSET,
        chapter_num
    };
}
