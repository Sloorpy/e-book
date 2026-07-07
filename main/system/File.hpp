#pragma once
#include "SDManager.hpp"
#include <vector>

class File final
{
public:
    explicit File(const std::string_view& filename,  const std::string mode);
    ~File();

public:
    std::string read_all();
    std::string read(const size_t size);
    std::vector<uint8_t> read_all_bytes();
    std::vector<uint8_t> read_bytes(const size_t size);
    void write(const std::string& txt);
    void seek(size_t position);
    uint64_t size();

private:
    FILE* open_file(const std::string_view& filename, const std::string mode);

private:    
    FILE* _fd;

private:
    static constexpr std::string_view LOG_TAG = "File";
};
