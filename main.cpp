#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <filesystem>

#include "Utils.h"
#include "indywv.h"
#include "labn.h"
#include "wav_writer.h"
#include "wav_format.h"
#include "unit_test.h"

#include <unordered_map>

const char* kInArg = "-in";
const char* kOutArg = "-out";
const char* kUnitTestArg = "-unit_test";

std::string get_filename_noext(const std::string& filepath)
{
    auto filename = filepath.substr(filepath.find_last_of("/\\") + 1);
    size_t lastindex = filename.find_last_of(".");
    return filename.substr(0, lastindex);
}

std::string get_file_ext(const std::string& filepath)
{
    auto filename = filepath.substr(filepath.find_last_of("/\\") + 1);
    size_t lastindex = filename.find_last_of(".");
    return filename.substr(lastindex + 1, filename.size() - lastindex - 1);
}

std::string getOutFolderPath(const std::string& outArg)
{
    namespace fs = std::filesystem;
    if (fs::is_directory(outArg))
        return fs::path(outArg).string();
    else
        return fs::path(outArg).parent_path().string();
}

std::string getOutFilePath(const std::string& inPath, const std::string& outArg, const std::string& outExt)
{
    std::string baseFileName = get_filename_noext(inPath);

    namespace fs = std::filesystem;
    if (fs::is_directory(outArg))
    {
        auto path = fs::path(outArg) / (baseFileName + "." + outExt);
        return path.string();
    }
    else
    {
        auto ext = get_file_ext(outArg);
        assert(ext == outExt);
        return outArg;
    }
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
        << "-out <FileOrFolderPath> : path of output file or folder\n"
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
    std::string outArg = result[kOutArg][0];

    std::ifstream file (inputFile, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return -1;

    file.seekg(0, std::ios::beg);

    char header[6];
    Utils::peekChar(file, header, 6);

    if (strncmp(header, LABN::kLABNId, 4) == 0)
    {
        auto outFolderPath = getOutFolderPath(outArg);
        LABN().decompressLabFile(inputFile, outFolderPath);
    }
    else if (strncmp(header, IndyWV::kIndyWV, 6) == 0)
    {
        auto outFilePath = getOutFilePath(inputFile, outArg, "wav");
        IndyWV().wv_to_wav(inputFile, outFilePath);
    }
    else if (strncmp(header, WavFormat::kRIFF, 4) == 0)
    {
        auto outFilePath = getOutFilePath(inputFile, outArg, "wv");
        IndyWV().wav_to_wv(inputFile, outFilePath);
    }
    else
    {
        std::cerr << "Unrecognized input file type\n";
    }
}
