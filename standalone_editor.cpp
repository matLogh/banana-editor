#include "TApplication.h"
#include "TFile.h"
#include "TH2.h"
#include "TList.h"
#include "TSystem.h"

#include "TBananaEditor.hh"

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

namespace
{
struct Options
{
    std::string inputFile;
    std::string matrixPath;
    std::string outputFile{"bananas.root"};
    std::string bananaPrefix{"banana"};
};

enum class ParseStatus
{
    Ok,
    Help,
    Error
};

void PrintUsage(const char *program)
{
    std::cout
        << "Usage:\n"
        << "  " << program
        << " --input <input.root> --matrix <directory/matrix> [options]\n"
        << "  " << program << " <input.root> <directory/matrix> [output.root] [prefix]\n\n"
        << "Options:\n"
        << "  -i, --input <file.root>    Input ROOT file\n"
        << "  -m, --matrix <path>        Full path to a TH2 inside the ROOT file\n"
        << "  -o, --output <file.root>   Cut file to create or edit (default: bananas.root)\n"
        << "  -p, --prefix <name>        Prefix for new cuts (default: banana)\n"
        << "  -h, --help                 Show this help message\n\n"
        << "Example:\n"
        << "  " << program
        << " -i sum.root -m AgataOscarUp/m_Energy_Theta -o C14.root -p C14_Ex2p7\n";
}

bool ReadOptionValue(int argc, char *argv[], int &index, std::string &destination)
{
    if (index + 1 >= argc)
    {
        std::cerr << "Missing value after " << argv[index] << '\n';
        return false;
    }
    destination = argv[++index];
    return true;
}

ParseStatus ParseCommandLine(int argc, char *argv[], Options &options)
{
    int positionalIndex = 0;
    for (int i = 1; i < argc; ++i)
    {
        const char *argument = argv[i];
        if (std::strcmp(argument, "-h") == 0 || std::strcmp(argument, "--help") == 0)
        {
            PrintUsage(argv[0]);
            return ParseStatus::Help;
        }
        if (std::strcmp(argument, "-i") == 0 || std::strcmp(argument, "--input") == 0)
        {
            if (!ReadOptionValue(argc, argv, i, options.inputFile))
                return ParseStatus::Error;
            continue;
        }
        if (std::strcmp(argument, "-m") == 0 || std::strcmp(argument, "--matrix") == 0)
        {
            if (!ReadOptionValue(argc, argv, i, options.matrixPath))
                return ParseStatus::Error;
            continue;
        }
        if (std::strcmp(argument, "-o") == 0 || std::strcmp(argument, "--output") == 0)
        {
            if (!ReadOptionValue(argc, argv, i, options.outputFile))
                return ParseStatus::Error;
            continue;
        }
        if (std::strcmp(argument, "-p") == 0 || std::strcmp(argument, "--prefix") == 0)
        {
            if (!ReadOptionValue(argc, argv, i, options.bananaPrefix))
                return ParseStatus::Error;
            continue;
        }
        if (argument[0] == '-')
        {
            std::cerr << "Unknown option: " << argument << '\n';
            return ParseStatus::Error;
        }

        switch (positionalIndex++)
        {
        case 0:
            options.inputFile = argument;
            break;
        case 1:
            options.matrixPath = argument;
            break;
        case 2:
            options.outputFile = argument;
            break;
        case 3:
            options.bananaPrefix = argument;
            break;
        default:
            std::cerr << "Unexpected positional argument: " << argument << '\n';
            return ParseStatus::Error;
        }
    }

    if (options.inputFile.empty() || options.matrixPath.empty())
    {
        std::cerr << "Both --input and --matrix are required\n";
        return ParseStatus::Error;
    }
    if (options.outputFile.empty() || options.bananaPrefix.empty())
    {
        std::cerr << "The output filename and banana prefix cannot be empty\n";
        return ParseStatus::Error;
    }
    if (options.inputFile == options.outputFile)
    {
        std::cerr << "Input and output ROOT files must be different\n";
        return ParseStatus::Error;
    }
    return ParseStatus::Ok;
}

bool EnsureOutputDirectory(const std::string &outputFile)
{
    const std::size_t separator = outputFile.find_last_of('/');
    if (separator == std::string::npos)
        return true;

    const std::string directory = outputFile.substr(0, separator);
    if (directory.empty())
        return true;

    if (gSystem->mkdir(directory.c_str(), kTRUE) != 0 &&
        gSystem->AccessPathName(directory.c_str()))
    {
        std::cerr << "Could not create output directory: " << directory << '\n';
        return false;
    }
    return true;
}

std::unique_ptr<TH2> LoadMatrix(const Options &options)
{
    TFile input(options.inputFile.c_str(), "READ");
    if (input.IsZombie())
    {
        std::cerr << "Cannot open input ROOT file: " << options.inputFile << '\n';
        return nullptr;
    }

    TH2 *source = input.Get<TH2>(options.matrixPath.c_str());
    if (!source)
    {
        std::cerr << "Cannot find TH2 matrix '" << options.matrixPath << "' in "
                  << options.inputFile << '\n';
        return nullptr;
    }

    auto matrix = std::unique_ptr<TH2>(dynamic_cast<TH2 *>(source->Clone("banana_editor_matrix")));
    if (!matrix)
    {
        std::cerr << "Could not clone matrix '" << options.matrixPath << "'\n";
        return nullptr;
    }
    matrix->SetDirectory(nullptr);
    return matrix;
}
} // namespace

int main(int argc, char *argv[])
{
    Options options;
    const ParseStatus parseStatus = ParseCommandLine(argc, argv, options);
    if (parseStatus == ParseStatus::Help)
        return 0;
    if (parseStatus == ParseStatus::Error)
    {
        PrintUsage(argv[0]);
        return 1;
    }

    if (!EnsureOutputDirectory(options.outputFile))
        return 1;

    std::unique_ptr<TH2> matrix = LoadMatrix(options);
    if (!matrix)
        return 1;

    TApplication application("Simple banana editor", nullptr, nullptr);
    TList         bananaList;
    TBananaEditor editor(("Banana editor: " + options.matrixPath).c_str());
    editor.SaveToFile(options.outputFile);
    editor.SetDefaultBananaName(options.bananaPrefix);
    editor.Init(nullptr, 1600, 900, matrix.get(), &bananaList, 0);

    return 0;
}
