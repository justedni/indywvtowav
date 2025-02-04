#include "utils.h"

#include <algorithm>

namespace Utils
{
    void peekChar(std::ifstream& stream, char* data, size_t size)
    {
        auto pos = stream.tellg();
        stream.read(data, size);
        stream.seekg(pos, std::ios::beg);
    }

    uint16_t swap16(uint16_t x)
    {
        uint16_t hi = (x & 0xff00) >> 8;
        uint16_t lo = (x & 0xff);
        return static_cast<uint16_t>(lo << 8) | hi;
    }
    int16_t swap16(int16_t x)
    {
        int16_t hi = (x & 0xff00) >> 8;
        int16_t lo = (x & 0xff);
        return static_cast<int16_t>(lo << 8) | hi;
    }

    std::string str_to_lower(const std::string& inputStr)
    {
        std::string retStr = inputStr;
        std::transform(retStr.begin(), retStr.end(), retStr.begin(),
            [](unsigned char c) { return std::tolower(c); });

        return retStr;
    }

} // namespace Utils
