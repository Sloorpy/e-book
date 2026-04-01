#pragma once
#include "SDManager.hpp"
#include <vector>

class File final
{
public:
    explicit File(const std::string_view& filename);
    ~File();

public:
    std::string read_all();
    std::string read(const uint32_t size);
    std::vector<uint8_t> read_all_bytes();
    std::vector<uint8_t> read_bytes(const uint32_t size);
    void seek(size_t position);

private:
    FILE* open_file(const std::string_view& filename);

private:    
    FILE* _fd;

private:
    static constexpr std::string_view LOG_TAG = "File";
};
