#include "wav_writer.h"

#include "indywv.h"

namespace WavWriter {

void write(std::string& path, const IndyWVHeader* wvHeader, char* inData)
{
    auto writeInt = [](std::ostream& os, auto val)
    {
        os.write(reinterpret_cast<const char*>(&val), sizeof(val));
    };

    std::ofstream os(path, std::ofstream::binary);
    os.write("RIFF", 4);
    writeInt(os, (uint32_t)(36 + wvHeader->decompressedSize));
    os.write("WAVE", 4);

    os.write("fmt ", 4);
    writeInt(os, (uint32_t)16);
    writeInt(os, (uint16_t)1);
    writeInt(os, (uint16_t)wvHeader->numChannels);
    writeInt(os, (uint32_t)wvHeader->sampleRate);
    uint16_t blockAlign = (wvHeader->numChannels * wvHeader->sampleBitSize / 8);
    writeInt(os, (uint32_t)(wvHeader->sampleRate * blockAlign));
    writeInt(os, (uint16_t)blockAlign);
    writeInt(os, (uint16_t)wvHeader->sampleBitSize);

    os.write("data", 4);
    writeInt(os, (uint32_t)(wvHeader->decompressedSize));

    os.write((char*)inData, wvHeader->decompressedSize);
}

} // namespace WavWriter