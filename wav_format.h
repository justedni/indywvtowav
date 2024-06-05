#pragma once

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

namespace WavFormat
{

constexpr static char kRIFF[6] = { 'R', 'I', 'F', 'F' };
constexpr static char kWAVE[4] = { 'W', 'A', 'V', 'E' };
constexpr static char kfmt[4] = { 'f', 'm', 't', ' ' };

PACK(struct WavHeader
{
    // RIFF header
    char tagRIFF[4];
    int wavSize;
    char tagWave[4];

    // Fmt header
    char tagFmt[4];
    int fmtChunkSize;
    short audioFormat;
    short numChannels;
    int sampleRate;
    int byteRate;
    short sampleAlignment;
    short bitDepth;

    // Data
    char tagData[4];
    int dataChunkSize;
});
static_assert(sizeof(WavHeader) == 44, "");

} // namespace WavFormat
