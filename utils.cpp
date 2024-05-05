#include "utils.h"

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

    float clamp(float val, float lower, float upper)
    {
        return std::max(lower, std::min(val, upper));
    }

} // namespace Utils
