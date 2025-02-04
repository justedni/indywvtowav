#include "labn.h"

#include "indywv.h"
#include "wave.h"

#include <vector>
#include <assert.h>
#include <iostream>

namespace LABN {

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

void decompress(const std::string& labPath, const std::string& outFolder)
{
    std::ifstream file(labPath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return;

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize))
        return;

    const auto* labHeaderPtr = reinterpret_cast<const LabHeader*>(buffer.data());
    assert(strncmp((char*)labHeaderPtr->id, "LABN", 4) == 0);

    const auto fileCount = labHeaderPtr->fileCount;
    const auto fileNameListLength = labHeaderPtr->fileNameListLength;

    const auto* labEntryPtr = reinterpret_cast<const LabFileEntry*>(labHeaderPtr + 1);
    const auto* labFileNameListPtr = reinterpret_cast<const char*>(labEntryPtr + labHeaderPtr->fileCount);
    std::string base_filename = labPath.substr(labPath.find_last_of("/\\") + 1);

    auto indyConverter = IndyWV();

    for (std::size_t f = 0; f < fileCount; ++f, ++labEntryPtr)
    {
        const auto& entry = *labEntryPtr;

        if (entry.nameOffset >= fileNameListLength)
        {
            std::cerr << "Warning: LAB entry with bad name offset! Ignoring it... " << base_filename << ".\n";
            continue;
        }
        if (entry.dataOffset >= fileSize)
        {
            std::cerr << "Warning: LAB entry with bad data offset! Ignoring it... " << base_filename << ".\n";
            continue;
        }
        if ((entry.dataOffset + entry.sizeInBytes) > fileSize)
        {
            std::cerr << "Warning: LAB entry with bad data offset/size! Ignoring it... " << base_filename << ".\n";
            continue;
        }

        const char* myData = (char*)labHeaderPtr + entry.dataOffset;
        const auto mySize = entry.sizeInBytes;

        std::string fileName{ &labFileNameListPtr[entry.nameOffset] };

        if (strncmp((char*)myData, IndyWV::kIndyWV, 6) == 0)
        {
            const auto* wvHeader = reinterpret_cast<const IndyWVHeader*>(myData);

            char* outBuffer = new char[wvHeader->decompressedSize];
            memset(outBuffer, 0, wvHeader->decompressedSize);

            size_t lastindex = fileName.find_last_of(".");
            auto fileNameNoExt = fileName.substr(0, lastindex);
            std::string outPath = outFolder + "\\" + fileNameNoExt + ".wav";

            file.seekg(entry.dataOffset + sizeof(IndyWVHeader), std::ios::beg);

            indyConverter.decompress(file, wvHeader->dataSize, outBuffer, wvHeader->decompressedSize);

            Wave::write(outPath, outBuffer,
                wvHeader->decompressedSize, wvHeader->numChannels, wvHeader->sampleRate, wvHeader->sampleBitSize);

            delete[] outBuffer;
        }
    }
}

} // namespace LABN
