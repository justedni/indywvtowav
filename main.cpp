#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>

#include "Utils.h"
#include "indywv.h"
#include "labn.h"
#include "wav_writer.h"
#include "unit_test.h"

#include <unordered_map>

const char* kInArg = "-in";
const char* kOutArg = "-out";
const char* kUnitTestArg = "-unit_test";

std::string get_filename_noext(std::string& filepath)
{
    auto filename = filepath.substr(filepath.find_last_of("/\\") + 1);
    size_t lastindex = filename.find_last_of(".");
    return filename.substr(0, lastindex);
}

using string_map = std::unordered_map<std::string, std::vector<std::string>>;
template <class Keyword>
string_map generic_parse(std::vector<std::string> as, Keyword keyword)
{
    string_map result;

    std::string flag;
    for (auto&& x : as)
    {
        auto f = keyword(x);
        if (f.empty())
        {
            result[flag].push_back(x);
        }
        else
        {
            flag = f.front();
            result[flag]; // Ensure the flag exists
            flag = f.back();
        }
    }
    return result;
}

void printUsage()
{
    std::cout << "Usage:\n"
        << "-in <FilePath> : full path of input file, INDYWV or LAB. Type will be auto-deduced)\n"
        << "-out <Folder> : path of output folder\n"
        << "[-unit_test] : performs unit test - checks algorithm integrity (optional)\n";
}

int main(int argc, const char* argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc);

    std::unordered_map<std::string, std::string> keys = {
        { kInArg, kInArg },
        { kOutArg, kOutArg },
        { kUnitTestArg, kUnitTestArg }
    };

    auto result = generic_parse(args, [&](auto&& s) -> std::vector<std::string> {
        if (keys.count(s) > 0)
            return { keys[s] };
        else
            return {};
    });

    if (result.find(kUnitTestArg) != result.end())
    {
        auto converter = IndyWV();

        std::string wvPath = "..\\UnitTest\\mono_adpcm_test.WV";
        std::string refFile = "..\\UnitTest\\mono_adpcm_test.wav";
        UnitTest::unit_test(wvPath, refFile);

        wvPath = "..\\UnitTest\\stereo_wvsm_test.wv";
        refFile = "..\\UnitTest\\stereo_wvsm_test.wav";
        UnitTest::unit_test(wvPath, refFile);
        return 0;
    }

    if (result.find(kInArg) == result.end() || result[kInArg].empty())
    {
        std::cerr << "Missing -in parameter\n";
        printUsage();
        return -1;
    }

    if (result.find(kOutArg) == result.end() || result[kOutArg].empty())
    {
        std::cerr << "Missing -out parameter\n";
        printUsage();
        return -1;
    }

    std::string inputFile = result[kInArg][0];
    std::string outFolder = result[kOutArg][0];

    std::ifstream file (inputFile, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return -1;

    file.seekg(0, std::ios::beg);

    char header[6];
    Utils::peekChar(file, header, 6);

    if (strncmp(header, LABN::kLABNId, 4) == 0)
    {
        LABN().decompressLabFile(inputFile, outFolder);
    }
    else if (strncmp(header, IndyWV::kIndyWV, 6) == 0)
    {
        std::string baseFileName = get_filename_noext(inputFile);
        std::string outFile = outFolder + "\\" + baseFileName + ".wav";

        IndyWV().wv_to_wav(inputFile, outFile);
    }
    else
    {
        std::cerr << "Unrecognized input file type\n";
    }
}
