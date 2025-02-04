#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <filesystem>

#include "Utils.h"
#include "indywv.h"
#include "labn.h"
#include "wave.h"
#include "unit_test.h"

#include "cryo_apc.h"

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

EFileType getFileTypeFromExt(const std::string& filePath)
{
    auto path = std::filesystem::path(filePath);
    auto ext = Utils::str_to_lower(path.extension().string());
    if (ext == ".lab") { return EFileType::LABN; }
    else if (ext == ".wv") { return EFileType::IndyWV; }
    else if (ext == ".wav") { return EFileType::Wave; }
    else if (ext == ".apc") { return EFileType::CryoAPC; }

    return EFileType::Unknown;
}

EFileType getFileType(std::ifstream& file)
{
    file.seekg(0, std::ios::beg);

    char header[8];
    Utils::peekChar(file, header, 8);

    if (strncmp(header, LABN::kLABNId, sizeof(LABN::kLABNId)) == 0)
    {
        return EFileType::LABN;
    }
    else if (strncmp(header, IndyWV::kIndyWV, sizeof(IndyWV::kIndyWV)) == 0)
    {
        return EFileType::IndyWV;
    }
    else if (strncmp(header, Wave::kRIFF, sizeof(Wave::kRIFF)) == 0)
    {
        return EFileType::Wave;
    }
    else if (strncmp(header, CryoAPC::kAPCTag, sizeof(CryoAPC::kAPCTag)) == 0)
    {
        return EFileType::CryoAPC;
    }

    return EFileType::Unknown;
}

bool convertFile(const std::string& inputPath, const std::string& outputArg)
{
    std::ifstream file(inputPath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return false;

    EFileType fileType = getFileTypeFromExt(inputPath);
    EFileType actualFileType = getFileType(file);
    assert(fileType == actualFileType);

    switch (fileType)
    {
    case EFileType::IndyWV:
    {
        auto outFilePath = getOutFilePath(inputPath, outputArg, "wav");
        IndyWV().wv_to_wav(inputPath, outFilePath);
        break;
    }
    case EFileType::LABN:
    {
        auto outFolderPath = getOutFolderPath(outputArg);
        LABN::decompress(inputPath, outFolderPath);
        break;
    }
    case EFileType::Wave:
    {
        auto outFilePath = getOutFilePath(inputPath, outputArg, "wv");
        IndyWV().wav_to_wv(inputPath, outFilePath);
        break;
    }
    case EFileType::CryoAPC:
    {
        auto outFilePath = getOutFilePath(inputPath, outputArg, "wav");
        CryoAPC::apc_to_wav(inputPath, outFilePath);
        break;
    }
    case EFileType::Unknown:
    default:
        std::cerr << "Unrecognized input file type\n";
    }
}

void do_unit_tests()
{
    struct Test
    {
        std::string name;
        std::string inFile;
        std::string outExt;
        std::string refFile;
    };

    std::vector<Test> tests{
        { "INDYWV (ADPCM) to WAV", "dice_mono_adpcm.wv", "wav", "dice_mono_adpcm.wav" },
        { "WAV (ADPCM) to INDYWV", "dice_mono_adpcm.wav", "wv", "dice_mono_adpcm.wv" },
        { "INDYWV (WVSM) to WAV", "stereo_wvsm_test.wv", "wav", "stereo_wvsm_test.wav" },
        { "APC (mono) to WAV", "toctoc_mono.apc", "wav", "toctoc_mono.wav" },
        { "APC (stereo) to WAV", "eboulis_stereo.apc", "wav", "eboulis_stereo.wav" },
    };

    namespace fs = std::filesystem;
    auto folder = fs::path("..\\..\\..\\src\\test_files");

    for (auto& test : tests)
    {
        auto inPath = folder / test.inFile;
        auto outPath = (folder / "temp").string() + "." + test.outExt;
        auto refFilePath = folder / test.refFile;
        convertFile(inPath.string(), outPath);
        UnitTest::unit_test(test.name, outPath, refFilePath.string());
        std::remove(outPath.c_str());
    }
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
        do_unit_tests();
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

    std::string inputPath = result[kInArg][0];
    std::string outArg = result[kOutArg][0];

    namespace fs = std::filesystem;
    auto inPath = std::filesystem::path(inputPath);
    if (fs::is_directory(inPath))
    {
        for (const auto& entry : std::filesystem::directory_iterator(inPath))
        {
            convertFile(entry.path().string(), outArg);
        }
    }
    else if (fs::is_regular_file(inPath))
    {
        convertFile(inputPath, outArg);
    }
}
