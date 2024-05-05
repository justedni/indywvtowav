#pragma once

#include <fstream>
#include <sstream>

namespace Utils
{
    uint16_t swap16(uint16_t x);
    int16_t swap16(int16_t x);

    float clamp(float val, float lower, float upper);

    void peekChar(std::ifstream& stream, char* data, size_t size);

    template<typename T>
    T readBytes(std::ifstream& stream)
    {
        T a;
        stream.read((char*)&a, sizeof(T));
        return a;
    }
}
