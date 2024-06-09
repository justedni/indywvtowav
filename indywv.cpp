#include "indywv.h"

#include <array>
#include <assert.h>
#include <stdint.h>
#include <sstream>
#include <vector>
#include <iostream>

#include "utils.h"
#include "wav_writer.h"
#include "wav_format.h"
#include "wv_writer.h"

// ----------------------------------------------------------------------------
// from ida_defs.h
#define LAST_IND(x,part_type)    (sizeof(x)/sizeof(part_type) - 1)
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
#  define LOW_IND(x,part_type)   LAST_IND(x,part_type)
#  define HIGH_IND(x,part_type)  0
#else
#  define HIGH_IND(x,part_type)  LAST_IND(x,part_type)
#  define LOW_IND(x,part_type)   0
#endif

#define BYTEn(x, n)   (*((uint8_t*)&(x)+n))
#define WORDn(x, n)   (*((uint16_t*)&(x)+n))
#define DWORDn(x, n)  (*((uint32_t*)&(x)+n))

#define BYTE1(x)   BYTEn(x,  1)         // byte 1 (counting from 0)

#define LOBYTE(x)  BYTEn(x,LOW_IND(x,uint8_t))
#define LOWORD(x)  WORDn(x,LOW_IND(x,uint16_t))
#define LODWORD(x) DWORDn(x,LOW_IND(x,uint32_t))

#define HIBYTE(x)  BYTEn(x,HIGH_IND(x,uint8_t))
// ----------------------------------------------------------------------------

IndyWV::IndyWV()
{
    // Build index data
    for (uint32_t i = 0; i < 0x40; ++i)
    {
        for (uint32_t j = 0; j < 0x59; ++j)
        {
            int16_t stepsize = m_stepSizes[j];
            int16_t acc = 0;

            for (uint32_t mask = 32; mask > 0; mask >>= 1)
            {
                if ((mask & i) != 0)
                    acc += stepsize;
                stepsize >>= 1;
            }

            auto offset = (64 * j + i);
            m_constructedIndexData[offset] = acc;
        }
    }
}

void IndyWV::wv_to_wav(std::string& in_WvPath, std::string& in_outFilePath)
{
    std::ifstream file(in_WvPath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return;

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize))
        return;

    const auto* wvHeader = reinterpret_cast<const IndyWVHeader*>(buffer.data());
    assert(strncmp((char*)wvHeader->tag, IndyWV::kIndyWV, 6) == 0);

    char* outBuffer = new char[wvHeader->decompressedSize];
    memset(outBuffer, 0, wvHeader->decompressedSize);

    file.seekg(sizeof(IndyWVHeader), std::ios::beg);

    inflate(file, wvHeader->dataSize, outBuffer, wvHeader->decompressedSize);
    WavWriter::write(in_outFilePath, wvHeader, outBuffer);

    delete[] outBuffer;
}

void IndyWV::wav_to_wv(std::string& in_WavPath, std::string& in_outFilePath)
{
    using namespace WavFormat;

    std::ifstream file(in_WavPath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return;

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize))
        return;

    const auto* wavHeader = reinterpret_cast<const WavHeader*>(buffer.data());
    assert(strncmp((char*)wavHeader->tagRIFF, kRIFF, 4) == 0);

    file.seekg(sizeof(WavHeader), std::ios::beg);

    auto dataToRead = wavHeader->dataChunkSize;
    char* inBuffer = new char[dataToRead];
    memset(inBuffer, 0, dataToRead);
    file.read(inBuffer, dataToRead);

    if (wavHeader->numChannels == 1)
    {
        // INDYWV ADPCM compression
        auto outSize = wavHeader->dataChunkSize / 2;
        char* outBuffer = new char[outSize];
        memset(outBuffer, 0, outSize);

        auto state = DecompressorState();
        auto compressedSize = compressADPCM(&state, outBuffer, inBuffer, outSize, wavHeader->numChannels);

        WvWriter::write(in_outFilePath, wavHeader, outBuffer, compressedSize);

        delete[] outBuffer;
    }
    else
    {
        // TODO handle WVSM compression
    }

    delete[] inBuffer;
}

void IndyWV::inflate(std::ifstream& istream, uint32_t inputDataSize, char* outBuffer, uint32_t infSize)
{ 
    using namespace Utils;

    std::size_t numChannels = 1;
    auto state = DecompressorState();

    auto unknownParam1 = readBytes<int8_t>(istream);
    auto unknownParam2 = readBytes<int16_t>(istream);

    if (unknownParam1 < 0)
    {
        state.stepindex[0] = ~unknownParam1;
        numChannels = 2;
    }

    state.keysample[0] = swap16(unknownParam2);
    if (numChannels > 1)
    {
        state.stepindex[1] = readBytes<int8_t>(istream);
        state.keysample[1] = swap16(readBytes<int16_t>(istream));
    }

    char WVSMHeader[4];
    peekChar(istream, WVSMHeader, 4);

    if (numChannels == 2 &&
        state.stepindex[1] == 0x64 &&
        state.keysample[0] == 0x1111 &&
        state.keysample[1] == 0x2222 &&
        strncmp(WVSMHeader, kWVSM, 4) == 0
        )
    {
        // WVSM decompression
        istream.read((char*)&WVSMHeader, 4); // advance

        constexpr std::size_t blockSize = 4096;
        short* outPtr = (short*)outBuffer;
        for (std::size_t i = 0; i < infSize / blockSize; i++)
        {
            wvsmInflateBlock(istream, blockSize, outPtr);
        }

        // Read the remaining data, shorter than 1 block
        wvsmInflateBlock(istream, infSize % blockSize, outPtr);

    }
    else
    {
        // ADPCM decompression
        auto dataToRead = inputDataSize;
        char* inBuffer = new char[dataToRead];
        memset(inBuffer, 0, dataToRead);
        istream.read(inBuffer, dataToRead);

        decompressADPCM(&state, outBuffer, inBuffer, infSize / (numChannels << 1), numChannels);

        delete[] inBuffer;
    }
}

void IndyWV::wvsmInflateBlock(std::ifstream& istream, std::size_t blockSize, short*& outData)
{
    using namespace Utils;

    std::size_t nSamples = blockSize / 2;
    if (nSamples == 0) {
        return;
    }

    auto compressedSize = swap16(readBytes<uint16_t>(istream)); // Note, big endian size
    const char se = readBytes<char>(istream); // sample expander
    const int sel = se >> 4;
    const int ser = se & 0xF;

    auto getChannelSample = [&](int expander) -> uint16_t
    {
        uint16_t val = readBytes<unsigned char>(istream);
        if (val == 0x80)
        {
            auto s = readBytes<uint16_t>(istream);
            val = swap16(s);
        }
        else {
            val = static_cast<int8_t>(val) << expander;
        }
        return val;
    };

    for (std::size_t i = 0; i < nSamples; i += 2)
    {
        auto val = getChannelSample(sel);
        *outData = val;
        outData++;
        if (i + 1 >= nSamples)
            return;

        val = getChannelSample(ser);
        *outData = val;
        outData++;
    }
}

void IndyWV::decompressADPCM(DecompressorState* compState, char* outData, char* in_data, int dataSize, unsigned int numChannels)
{
    int accStep = 0;

    unsigned __int16 inDataSwap = _byteswap_ushort(*(uint16_t*)in_data);
    char* pInDataStart = in_data + 2;
    for (uint32_t k = 0; k < numChannels; ++k)
    {
        short* pOutData = (short*)&outData[2 * k];
        int8_t lastIndex = compState->stepindex[k];
        int16_t lastData = compState->keysample[k];
        char* pCurrInData = pInDataStart;
        int remainingData = dataSize;
        while (remainingData)
        {
            unsigned char step = m_steps[lastIndex];
            int stepshift = 1 << (step - 1);
            char tempOffset = stepshift - 1;
            accStep += step;
            int temp1 = stepshift;
            LOBYTE(temp1) = (stepshift - 1) | stepshift;
            int offset = temp1 & (inDataSwap >> (16 - accStep));
            if (accStep > 7)
            {
                accStep -= 8;
                unsigned short temp2 = inDataSwap << 8;
                LOBYTE(temp2) = *pCurrInData;
                inDataSwap = temp2;
                ++pCurrInData;
            }
            if ((offset & stepshift) != 0)
                offset ^= stepshift;
            else
                LOWORD(stepshift) = 0;

            int prediction = 0;
            if ((uint8_t)offset == tempOffset)
            {
                auto calc = inDataSwap;
                prediction = (int16_t)(inDataSwap << accStep);
                LOWORD(calc) = inDataSwap << 8;
                LOBYTE(calc) = *pCurrInData;
                LOBYTE(prediction) = calc >> (8 - accStep);
                LOWORD(calc) = (uint16_t)calc << 8;
                LOBYTE(calc) = *(pCurrInData + 1);
                pCurrInData = pCurrInData + 2;
                inDataSwap = calc;
            }
            else
            {
                auto tableOffset = (lastIndex << 6) | (offset << (7 - step));
                int dataOffset = *(uint16_t*)&m_constructedIndexData[tableOffset];
                if ((uint16_t)offset)
                {
                    dataOffset += (uint32_t)m_stepSizes[lastIndex] >> (step - 1);
                }

                if (stepshift > 0)
                    prediction = std::max(lastData - dataOffset, -32768);
                else
                    prediction = std::min(lastData + dataOffset, 32767);
            }
            lastData = prediction;
            *pOutData = prediction;
            pOutData += numChannels;

            int index = getIndex(step * 4, offset) + lastIndex;
            index = Utils::clamp(index, 0, 88);
            lastIndex = index;
            --remainingData;
        }
        pInDataStart = pCurrInData;
        numChannels = numChannels & 0x7fffffff;

        compState->stepindex[k] = lastIndex;
        compState->keysample[k] = lastData;
    }
}

int IndyWV::compressADPCM(DecompressorState* compState, char* outData, char* inData, int sndDataSize, unsigned int numChannels)
{
    if (numChannels == 0)
        return 0;

    int v9 = 0;
    char accStep = 0;
    char* pOutData = outData;
    int v38 = 0;

    for (uint8_t iChan = 0; iChan < numChannels; iChan++)
    {
        char* pInData = inData;
        int lastIndex = *(char*)(compState + iChan);
        int lastData = compState->keysample[iChan];
        int v30 = lastData;

        int remainingData = sndDataSize;
        while (remainingData)
        {
            int v32 = 0;
            int offset = 0;
            uint16_t v13 = m_stepSizes[lastIndex];
            char initialized = 0;
            int v14 = *(__int16*)pInData;
            int v17 = v14 - lastData;
            unsigned char step = m_steps[lastIndex];
            int stepshift = 1 << (step - 1);
            char tempOffset = stepshift - 1;
            if (v17 < 0)
            {
                initialized = 1 << ((step - 1) & 0x1f);
                v17 = -v17;
            }
            int v20 = stepshift >> 1;
            int v21 = step - 1;
            if (step != 1)
            {
                for (int iStep = step - 1; iStep > 0; --iStep)
                {
                    if (v17 >= v13)
                    {
                        v17 -= v13;
                        offset |= v20;
                        v21 = v13 + v32;
                        v32 += v13;
                    }
                    v13 >>= 1;
                    v20 >>= 1;
                }
            }
            if (offset)
                v32 += v13;
            unsigned __int8 v23 = 8 - (accStep & 7);
            LOWORD(v21) = (uint8_t)(offset | initialized);
            v9 = (v38 << step) | v21;
            v38 = v9;
            accStep += step;
            if (step >= v23)
                *pOutData++ = (uint16_t)v9 >> (step - v23);
            if (offset == tempOffset)
            {
                __int16 v24 = *(__int16*)pInData;
                unsigned __int16 v26;
                LOBYTE(v26) = BYTE1(v24);
                HIBYTE(v26) = v9;
                char* v27 = pOutData + 1;
                *pOutData = v26 >> (accStep & 7);
                v9 = (unsigned __int16)v24;
                v38 = (unsigned __int16)v24;
                pOutData += 2;
                *v27 = (unsigned __int16)v24 >> (accStep & 7);

                lastData = v24;
            }
            else
            {
                lastData += initialized ? -v32 : v32;
                lastData = Utils::clamp(lastData, -32768, 32767);
            }

            lastIndex += getIndex(step * 4, offset);
            lastIndex = Utils::clamp(lastIndex, 0, 88);

            pInData += 2;
            --remainingData;
        }

        *(unsigned char*)(iChan + compState) = lastIndex;
        compState->keysample[iChan] = lastData;
        inData += 2;
    }

    if ((accStep & 7) != 0)
        *pOutData++ = v9 << (8 - (accStep & 7));

    int writtenBytes = (pOutData - outData);
    return writtenBytes + 4;
}
