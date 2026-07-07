#include "File.hpp"
#include <esp_log.h>
#include <stdexcept>
#include <string>

File::File(const std::string_view& filename, const std::string mode) :  
    _fd(open_file(filename, mode)) 
{}

File::~File()
{
    if (_fd)
    {
        fclose(_fd);
    }
}

std::string File::read_all()
{
    std::vector<uint8_t> buffer = read_all_bytes();
    return std::string(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

std::string File::read(const size_t size)
{
    std::vector<uint8_t> buffer = read_bytes(size);
    return std::string(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

std::vector<uint8_t> File::read_all_bytes()
{
    static constexpr size_t BUFFER_SIZE = 1024;

    std::vector<uint8_t> output;
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    size_t bytes_read = fread(buffer.data(), sizeof(uint8_t), buffer.size(), _fd);

    while (bytes_read)
    {
        output.insert(output.end(), buffer.begin(), buffer.begin() + bytes_read);
        
        if (bytes_read != buffer.size())
        {
            break;
        }

        bytes_read = fread(buffer.data(), sizeof(uint8_t), buffer.size(), _fd);
    }

    if (ferror(_fd))
    {
        ESP_LOGE(LOG_TAG.data(), "Error reading file");
        return {};
    }

    output.shrink_to_fit();
    return output;
}

std::vector<uint8_t> File::read_bytes(const size_t size)
{
    std::vector<uint8_t> buffer(size);

    const size_t bytes_read = fread(buffer.data(), sizeof(uint8_t), size, _fd);
    if (bytes_read != size && ferror(_fd))
    {
        ESP_LOGE(LOG_TAG.data(), "Error reading file");
        return {};
    }

    buffer.resize(bytes_read);
    return buffer;
}

void File::write(const std::string &txt)
{
    const size_t bytes_written = fwrite(txt.data(), sizeof(char), txt.size(), _fd);

    if (bytes_written !=  txt.size())
    {
        ESP_LOGE(LOG_TAG.data(),
                 "Failed to write file. Expected %zu bytes, wrote %zu bytes",
                  txt.size(),
                 bytes_written);
        throw std::runtime_error("Failed to write full file");
    }

    if (fflush(_fd) != 0)
    {
        ESP_LOGE(LOG_TAG.data(), "Failed to flush file after write");
        throw std::runtime_error("Failed to flush file");
    }
}

void File::seek(size_t position) {
    int result = fseek(_fd, static_cast<long>(position), SEEK_SET);
    
    if (result != 0)
    {
        ESP_LOGE(LOG_TAG.data(), "Failed to seek to %zu", position);
        throw std::runtime_error("Failed to seek file");
    }
}

uint64_t File::size()
{
    const long initial_pos = ftell(_fd);
    if (initial_pos < 0) {
        throw std::runtime_error("Failed to get file position");
    }

    if (fseek(_fd, 0, SEEK_END) != 0) {
        throw std::runtime_error("Failed to seek file end");
    }    
    
    const long file_size = ftell(_fd);
    if (file_size < 0) {
        throw std::runtime_error("Failed to get file size");
    }
    
    if (fseek(_fd, initial_pos, SEEK_SET) != 0) {
        throw std::runtime_error("Failed to restore file position");
    }

    return static_cast<uint64_t>(file_size);
}

FILE *File::open_file(const std::string_view &filename, const std::string mode)
{
    const std::string file_path = std::string(SDManager::instance().get_base_path()) + "/" + std::string(filename);
    FILE* fd = fopen(file_path.c_str(), mode.c_str());
    
    if (!fd) 
    {
        ESP_LOGE(LOG_TAG.data(), "Failed to open %s for reading", file_path.c_str());
        throw std::runtime_error("Failed to open " + file_path + " for reading");
    }

    return fd;
}
