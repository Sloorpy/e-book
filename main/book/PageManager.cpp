#include "PageManager.hpp"
#include <esp_log.h>
#include <string>
static constexpr char TAG[] = "PageManager";


PageManager::PageManager(const std::string_view &book_name, std::shared_ptr<SDManager> sd) :
    _book_name(book_name),
    _sd(sd),
    _chapters_count(0)
{
    _chapters_count = chapter_count();
}

std::string PageManager::get_cover() const
{
    return File(_sd, book_file_path("cover")).read_all();
}

std::string PageManager::get_chapter_title(const uint16_t chapter_num) const
{
    return File(_sd, chapter_file_path(chapter_num, "chapter")).read_all();
}

uint16_t PageManager::chapter_count() const
{
    if (_chapters_count > 0) {
        return _chapters_count;
    }

    const std::string chapter_count_path = book_file_path("chapter_count");
    const std::string chapter_count = File(_sd, chapter_count_path).read_all();

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

Chapter PageManager::load_chapter(const uint16_t chapter_num)
{
    if (chapter_num > chapter_count())
    {
        throw std::runtime_error("Error: Tried to get invalid chapter");
    }

    const std::string pages_path = chapter_file_path(chapter_num, "pages");
    const std::string chapter_path = chapter_file_path(chapter_num, "chapter");

    static constexpr size_t START_OFFSET = 0;
    return Chapter{
        File(_sd, pages_path).read_all(),
        File(_sd, chapter_path).read_all(),
        START_OFFSET,
        chapter_num
    };;
}
