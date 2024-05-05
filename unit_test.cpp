#include "unit_test.h"

#include "indywv.h"
#include "wav_writer.h"

#include <vector>
#include <assert.h>
#include <iostream>

#include <cstdlib>

void UnitTest::unit_test(std::string& in_WvPath, std::string& in_referenceWavFilePath)
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

    IndyWV().inflate(file, wvHeader->dataSize, outBuffer, wvHeader->decompressedSize);

    std::string convertedFilePath = in_WvPath + "_temp";
    WavWriter::write(convertedFilePath, wvHeader, outBuffer);

    // Unit test
    auto bSuccess = [&]()
    {
        std::ifstream fileA(convertedFilePath, std::ios::in | std::ios::binary | std::ifstream::ate);
        std::ifstream fileB(in_referenceWavFilePath, std::ios::in | std::ios::binary | std::ifstream::ate);

        if (fileA.fail() || fileB.fail())
            return false;

        if (fileA.tellg() != fileB.tellg())
            return false;

        fileA.seekg(0, std::ifstream::beg);
        fileB.seekg(0, std::ifstream::beg);
        auto bAreEqual = std::equal(std::istreambuf_iterator<char>(fileA.rdbuf()),
            std::istreambuf_iterator<char>(),
            std::istreambuf_iterator<char>(fileB.rdbuf()));

        return bAreEqual;
    }();

    if (bSuccess)
        std::cout << "Unit Test: Success!\n";
    else
    {
        std::cerr << "Unit Test: Failed!\n";
#ifdef _DEBUG
        assert(bSuccess, "Unit test failed!");
#endif
    }

    std::remove(convertedFilePath.c_str());
    delete[] outBuffer;
}