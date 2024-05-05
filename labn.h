#pragma once

#include <stdint.h>
#include <string>

class LABN
{
public:
    constexpr static char kLABNId[4] = { 'L', 'A', 'B', 'N' };

    void decompressLabFile(const std::string& labPath, const std::string& outFolder);

private:
    struct LabHeader
    {
        uint8_t  id[4];
        uint32_t unknown;
        uint32_t fileCount;
        uint32_t fileNameListLength;
    };

    struct LabFileEntry
    {
        uint32_t nameOffset;
        uint32_t dataOffset;
        uint32_t sizeInBytes;
        uint8_t  typeId[4];
    };
};
