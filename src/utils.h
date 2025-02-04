#pragma once

#include <fstream>
#include <sstream>

namespace Utils
{
    uint16_t swap16(uint16_t x);
    int16_t swap16(int16_t x);

    template<typename T>
    T clamp(T val, T lower, T upper)
    {
        return std::max(lower, std::min(val, upper));
    }

    void peekChar(std::ifstream& stream, char* data, size_t size);

    std::string str_to_lower(const std::string& inputStr);

    template<typename T>
    T readBytes(std::ifstream& stream)
    {
        T a;
        stream.read((char*)&a, sizeof(T));
        return a;
    }

    template<typename T>
    auto writeInt(std::ostream& os, T val)
    {
        os.write(reinterpret_cast<const char*>(&val), sizeof(val));
    };

} // namespace Utils
