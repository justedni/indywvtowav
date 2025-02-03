#include "wave.h"

#include "utils.h"
#include <fstream>

namespace Wave {

constexpr static char kWAVE[4] = { 'W', 'A', 'V', 'E' };
constexpr static char kfmt[4] = { 'f', 'm', 't', ' ' };
constexpr static char kdata[4] = { 'd', 'a', 't', 'a' };

void write(const std::string& path, char* pData, uint32_t dataSize, uint8_t numChannels, uint32_t sampleRate, uint32_t bitSize)
{
    using namespace Utils;

    std::ofstream os(path, std::ofstream::binary);
    os.write(kRIFF, 4);
    writeInt(os, (uint32_t)(36 + dataSize));
    os.write(kWAVE, 4);

    os.write(kfmt, 4);
    writeInt(os, (uint32_t)16);
    writeInt(os, (uint16_t)1);
    writeInt(os, (uint16_t)numChannels);
    writeInt(os, (uint32_t)sampleRate);
    uint16_t blockAlign = (numChannels * bitSize / 8);
    writeInt(os, (uint32_t)(sampleRate * blockAlign));
    writeInt(os, (uint16_t)blockAlign);
    writeInt(os, (uint16_t)bitSize);

    os.write(kdata, 4);
    writeInt(os, (uint32_t)(dataSize));

    os.write((char*)pData, dataSize);
}

} // namespace Wave
