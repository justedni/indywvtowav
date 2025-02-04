#include "cryo_apc.h"

#include <fstream>
#include <vector>
#include <assert.h>

#include "common.h"
#include "utils.h"
#include "wave.h"

typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint8_t BYTE;

#define HINIBBLE(byte) ((byte) >> 4)
#define LONIBBLE(byte) ((byte) & 0x0F)

namespace CryoAPC {

PACK(struct APCHeader
{
    char szID[8];
    char szVersion[4];
    DWORD dwOutSize;
    DWORD dwSampleRate;
    LONG lSampleLeft;
    LONG lSampleRight;
    DWORD dwStereo;
});

LONG IndexAdjust[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

LONG StepTable[89] = {
   7,     8,	  9,	 10,	11,    12,     13,    14,    16,
   17,    19,	  21,	 23,	25,    28,     31,    34,    37,
   41,    45,	  50,	 55,	60,    66,     73,    80,    88,
   97,    107,   118,	 130,	143,   157,    173,   190,   209,
   230,   253,   279,	 307,	337,   371,    408,   449,   494,
   544,   598,   658,	 724,	796,   876,    963,   1060,  1166,
   1282,  1411,  1552,  1707,	1878,  2066,   2272,  2499,  2749,
   3024,  3327,  3660,  4026,	4428,  4871,   5358,  5894,  6484,
   7132,  7845,  8630,  9493,	10442, 11487,  12635, 13899, 15289,
   16818, 18500, 20350, 22385,  24623, 27086,  29794, 32767
};

//-----------------------------------------------------------------------------
// Implemented/cleaned up the algorithm found here:
// https://wiki.multimedia.cx/index.php/CRYO_APC
//-----------------------------------------------------------------------------
void apc_to_wav(const std::string& in_apcFilePath, const std::string& in_outFilePath)
{
    std::ifstream file(in_apcFilePath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return;

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize))
        return;

    const auto* header = reinterpret_cast<const APCHeader*>(buffer.data());
    assert(strncmp((char*)header->szID, kAPCTag, sizeof(kAPCTag)) == 0);

    uint8_t numChannels = header->dwStereo ? 2 : 1;
    uint32_t outBufferSize = header->dwOutSize * numChannels;

    uint16_t* outBuffer = new uint16_t[outBufferSize];
    memset(outBuffer, 0, outBufferSize);

    LONG lIndexLeft = 0, lIndexRight = 0;
    LONG lCurSampleLeft = header->lSampleLeft;
    LONG lCurSampleRight = header->lSampleRight;

    auto processNibble = [](BYTE Code, LONG& lIndex, LONG& lCurSample)
    {
        LONG Delta = StepTable[lIndex] >> 3;

        if (Code & 4)
            Delta += StepTable[lIndex];
        if (Code & 2)
            Delta += StepTable[lIndex] >> 1;
        if (Code & 1)
            Delta += StepTable[lIndex] >> 2;
        if (Code & 8) // sign bit
            lCurSample -= Delta;
        else
            lCurSample += Delta;

        // clip sample
        lCurSample = Utils::clamp(lCurSample, -32768, 32767);
        // adjust index
        lIndex += IndexAdjust[Code];
        // clip index
        lIndex = Utils::clamp(lIndex, 0, 88);
    };

    char* pData = buffer.data() + sizeof(APCHeader);
    uint16_t* outData = outBuffer;

    size_t physicalFileSize = fileSize - sizeof(APCHeader);
    size_t headerFileSize = (numChannels == 2) ? header->dwOutSize : (header->dwOutSize / 2);
    size_t remainingData = std::min(physicalFileSize, headerFileSize);

    while (remainingData)
    {
        BYTE Input = *pData; // current byte of compressed data

        // High nibble (common to mono and stereo)
        processNibble(HINIBBLE(Input), lIndexLeft, lCurSampleLeft);
        *outData++ = (uint16_t)lCurSampleLeft;

        // Low nibble: next sample for mono, right channel for stereo
        if (numChannels == 1)
        {
            processNibble(LONIBBLE(Input), lIndexLeft, lCurSampleLeft);
            *outData++ = (uint16_t)lCurSampleLeft;
        }
        else
        {
            processNibble(LONIBBLE(Input), lIndexRight, lCurSampleRight);
            *outData++ = (uint16_t)lCurSampleRight;
        }

        pData++;
        remainingData--;
    }

    Wave::write(in_outFilePath, (char*)outBuffer, outBufferSize * sizeof(uint16_t), numChannels, header->dwSampleRate, 16);

    delete[] outBuffer;
}

} // namespace CryoAPC
