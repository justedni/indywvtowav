#include "unit_test.h"

#include "indywv.h"
#include "wav_writer.h"

#include <vector>
#include <assert.h>
#include <iostream>

#include <cstdlib>

void UnitTest::unit_test(const std::string& testName, const std::string& in_leftFilePath, const std::string& in_rightFilePath)
{
    auto bSuccess = [&]()
    {
        std::ifstream fileA(in_leftFilePath, std::ios::in | std::ios::binary | std::ifstream::ate);
        std::ifstream fileB(in_rightFilePath, std::ios::in | std::ios::binary | std::ifstream::ate);

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

    std::cout << "Unit Test: " << testName << "...";

    if (bSuccess)
        std::cout << "Success!\n";
    else
    {
        std::cerr << "Failed!\n";
#ifdef _DEBUG
        assert(bSuccess, "Unit test failed!");
#endif
    }
}