#pragma once

#include <fstream>

#include "common.h"

namespace Wave {
    struct WavHeader;
}

PACK(struct IndyWVHeader
{
    const char tag[6];
    uint32_t sampleRate = 0;
    uint32_t sampleBitSize = 0;
    uint32_t numChannels = 0;
    uint32_t dataSize = 0;
    int32_t unknown = 0;
    int32_t decompressedSize = 0;
});
static_assert(sizeof(IndyWVHeader) == 30, "");

class IndyWV
{
public:
    constexpr static char kIndyWV[6] = { 'I', 'N', 'D', 'Y', 'W', 'V' };
    constexpr static char kWVSM[4] = { 'W', 'V', 'S', 'M' };

    IndyWV();

    struct DecompressorState
    {
        // state for each channel (mono or stereo left/right)
        int8_t  stepindex[2];
        int16_t keysample[2];
    };

    void wv_to_wav(const std::string& in_WvPath, std::string& in_outFilePath);
    void wav_to_wv(const std::string& in_WavPath, std::string& in_outFilePath);

    void write_wv_file(std::string& path, const Wave::WavHeader* wavHeader, char* inData, uint32_t compressedSize);

    void decompress(std::ifstream& istream, uint32_t inputDataSize, char* outBuffer, uint32_t infSize);

private:
    void wvsmInflateBlock(std::ifstream& istream, std::size_t blockSize, short*& outData);
    void decompressADPCM(DecompressorState* compState, char* outData, char* in_data, int dataSize, unsigned int numChannels);

    int compressADPCM(DecompressorState* compState, char* outData, char* in_data, int dataSize, unsigned int numChannels);

    static const char* aIndexTableTable[8];

    static const unsigned short aStepTable[89];
    static const char aStepBits[96];

    static short aDeltaTable[5696];
};
