#include "wv_writer.h"
#include "wav_format.h"

#include "indywv.h"

namespace WvWriter {

void write(std::string& path, const WavFormat::WavHeader* wavHeader, char* inData, uint32_t compressedSize)
{
    auto writeInt = [](std::ostream& os, auto val)
    {
        os.write(reinterpret_cast<const char*>(&val), sizeof(val));
    };

    std::ofstream os(path, std::ofstream::binary);
    os.write(IndyWV::kIndyWV, sizeof(IndyWV::kIndyWV));
    writeInt(os, (uint32_t)(wavHeader->sampleRate));
    writeInt(os, (uint32_t)(wavHeader->bitDepth));
    writeInt(os, (uint32_t)(wavHeader->numChannels));
    writeInt(os, (uint32_t)(compressedSize));
    writeInt(os, (int32_t)0);
    writeInt(os, (int32_t)wavHeader->dataChunkSize);
    os.write((char*)inData, compressedSize);
}

} // namespace WvWriter
