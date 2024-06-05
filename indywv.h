#pragma once

#include <fstream>

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

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

    void wv_to_wav(std::string& in_WvPath, std::string& in_outFilePath);
    void wav_to_wv(std::string& in_WavPath, std::string& in_outFilePath);

protected:
    void inflate(std::ifstream& istream, uint32_t inputDataSize, char* outBuffer, uint32_t infSize);
    void compress(std::ifstream& istream, uint32_t inputDataSize, char* outBuffer, uint32_t infSize);
    friend class UnitTest;
    friend class LABN;

private:
    void wvsmInflateBlock(std::ifstream& istream, std::size_t blockSize, short*& outData);
    void decompressADPCM(DecompressorState* compState, char* outData, char* in_data, int dataSize, unsigned int numChannels);

    int compressADPCM(DecompressorState* compState, char* outData, char* in_data, int dataSize, unsigned int numChannels);

    char getIndex(int addr, int offset);

    static const unsigned short m_stepSizes[89];
    static const char m_steps[96];

    short m_constructedIndexData[5696];
};
