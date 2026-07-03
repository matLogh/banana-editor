#include "TApplication.h"
#include "TButton.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH2D.h"
#include "TInterpreter.h"
#include "TPad.h"
#include "TRint.h"
#include "TSystem.h"

#include "TBananaEditor.hh"

#include <cstring>
#include <iostream>
#include <string>

std::string INPUT_ROOT_FILENAME{"example_data/oscar_sum.root"};
std::string OUTPUT_PATH{"."};

void PrintUsage(const char *program_name)
{
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  -i, --input <file.root>  Input ROOT file with OSCAR histograms\n"
              << "  -o, --output <directory> Output directory for saved banana ROOT files\n"
              << "  -h, --help              Show this help message\n";
}

enum class ParseStatus
{
    Ok,
    Help,
    Error
};

ParseStatus ParseCommandLine(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];

        if (std::strcmp(arg, "-h") == 0 || std::strcmp(arg, "--help") == 0)
        {
            PrintUsage(argv[0]);
            return ParseStatus::Help;
        }

        if (std::strcmp(arg, "-i") == 0 || std::strcmp(arg, "--input") == 0)
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value after " << arg << '\n';
                PrintUsage(argv[0]);
                return ParseStatus::Error;
            }
            INPUT_ROOT_FILENAME = argv[++i];
            continue;
        }

        if (std::strcmp(arg, "-o") == 0 || std::strcmp(arg, "--output") == 0)
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value after " << arg << '\n';
                PrintUsage(argv[0]);
                return ParseStatus::Error;
            }
            OUTPUT_PATH = argv[++i];
            continue;
        }

        if (arg[0] == '-')
        {
            std::cerr << "Unknown option: " << arg << '\n';
            PrintUsage(argv[0]);
            return ParseStatus::Error;
        }

        if (argc == 2)
        {
            INPUT_ROOT_FILENAME = arg;
            continue;
        }

        std::cerr << "Unexpected positional argument: " << arg << '\n';
        PrintUsage(argv[0]);
        return ParseStatus::Error;
    }

    return ParseStatus::Ok;
}

bool EnsureOutputDirectory()
{
    if (OUTPUT_PATH.empty())
    {
        std::cerr << "Output directory cannot be empty\n";
        return false;
    }

    if (gSystem->mkdir(OUTPUT_PATH.c_str(), kTRUE) != 0 && gSystem->AccessPathName(OUTPUT_PATH.c_str()))
    {
        std::cerr << "Could not create output directory: " << OUTPUT_PATH << '\n';
        return false;
    }

    void *directory = gSystem->OpenDirectory(OUTPUT_PATH.c_str());
    if (!directory)
    {
        std::cerr << "Output path is not a directory: " << OUTPUT_PATH << '\n';
        return false;
    }
    gSystem->FreeDirectory(directory);

    return true;
}

TH2 *ReadTelescopeHistogram(TFile &file_data, int det_id)
{
    const std::string hist_name = "m_dE_E_tel" + std::to_string(det_id);

    const char *up_directories[] = {"up_blu", "OscarUp/EdE"};
    const char *down_directories[] = {"up_nero", "down_nero", "OscarDown/EdE"};

    const char **directories = det_id < 64 ? up_directories : down_directories;
    const int directory_count = det_id < 64 ? 2 : 3;

    for (int i = 0; i < directory_count; ++i)
    {
        const std::string object_path = std::string(directories[i]) + "/" + hist_name;
        TH2 *matrix = dynamic_cast<TH2 *>(file_data.Get(object_path.c_str()));
        if (matrix)
        {
            matrix->SetDirectory(nullptr);
            return matrix;
        }
    }

    return nullptr;
}

extern "C" void plot_telescope(int det_id, const char *input_root_filename)
{
    TFile file_data(input_root_filename, "READ");
    if (file_data.IsZombie())
    {
        std::cerr << "Cannot open input ROOT file: " << input_root_filename << '\n';
        return;
    }

    TH2 *matrix = ReadTelescopeHistogram(file_data, det_id);
    file_data.Close();
    if (!matrix)
    {
        std::cerr << "Histogram m_dE_E_tel" << det_id << " was not found in " << input_root_filename
                  << ". Checked up_blu/OscarUp/EdE for telescopes 0-63 and "
                  << "up_nero/down_nero/OscarDown/EdE for telescopes 64-127.\n";
        return;
    }

    TList banana_list;

    TBananaEditor editor(Form("Banana editor: OSCAR pseudotelescope %i", det_id));
    editor.SetDefaultBananaName("a14_z6_t" + std::to_string(det_id));

    std::string fname = OUTPUT_PATH + "/oscar_t_" + std::to_string(det_id) + ".root";
    editor.SaveToFile(fname.c_str());
    editor.Init(nullptr, 1600, 800, matrix, &banana_list, 0);
    delete matrix;
}

void CreateCanvasWithButtons()
{
    // Create a new canvas
    TCanvas *canvas = new TCanvas("Oscar", "OSCAR pseudotelescope map", 800, 450);
    // Closing the selector window should end the ROOT application loop.
    if (gApplication)
    {
        canvas->Connect("Closed()", "TApplication", gApplication, "Terminate()");
    }

    // Divide the canvas into 64 pads (16x4) + a gap (16x2) + 64 pads (16x4)
    // blu
    const int ncol = 16;
    const int nrow = 9;
    const int nrow_telescope = 4;
    canvas->Divide(ncol, nrow);

    // Loop over the pads
    // blu
    for (int x = 0; x < ncol; x++)
    {
        for (int y = 0; y < nrow_telescope; y++)
        {
            int button_pos = x + ncol * y;
            int det_id = x + ncol * y;
            // Set the pad's unique (x,y) position
            auto *pad = canvas->cd(button_pos + 1);
            pad->SetTopMargin(0);
            pad->SetBottomMargin(0);
            pad->SetLeftMargin(0);
            pad->SetRightMargin(0);
            // pad->SetCursor(kHand);
            // Create a button on the pad
            TButton *button =
                new TButton(Form("%i", det_id), Form("plot_telescope(%i,\"%s\")", det_id, INPUT_ROOT_FILENAME.c_str()),
                            0.0, 0.0, 1.0, 1.0);
            button->SetFillColor(kBlue);
            button->SetTextColor(kWhite);

            if (x == 2 || x == 6)
            {
                button->SetFillColor(kGray);
                button->SetTextColor(kGray);
                button->SetBit(TObject::kCannotPick); // Keep visible, but ignore mouse interaction
                // pad->SetCursor(kCross);
            }
            button->SetTextSize(0.5);
            button->Draw();
        }
    }

    // nero
    for (int x = 0; x < ncol; x++)
    {
        for (int y = nrow_telescope + 1; y < nrow; y++)
        {
            int button_pos = x + ncol * y;
            int det_id = 64 + x + ncol * (y - nrow_telescope - 1);
            // Set the pad's unique (x,y) position
            auto *pad = canvas->cd(button_pos + 1);
            pad->SetTopMargin(0);
            pad->SetBottomMargin(0);
            pad->SetLeftMargin(0);
            pad->SetRightMargin(0);
            // pad->SetCursor(kHand);
            // Create a button on the pad
            TButton *button =
                new TButton(Form("%i", det_id), Form("plot_telescope(%i,\"%s\")", det_id, INPUT_ROOT_FILENAME.c_str()),
                            0.0, 0.0, 1.0, 1.0);
            button->SetFillColor(kBlack);
            button->SetTextColor(kWhite); // White text for black buttons
            button->SetTextSize(0.5);
            button->Draw();
        }
    }

    // Update the canvas
    canvas->Update();
    canvas->SetEditable(kFALSE);
}

int main(int argc, char *argv[])
{
    const ParseStatus parse_status = ParseCommandLine(argc, argv);
    if (parse_status == ParseStatus::Help)
    {
        return 0;
    }
    if (parse_status == ParseStatus::Error)
    {
        return 1;
    }
    if (!EnsureOutputDirectory())
    {
        return 1;
    }

    TApplication app("Banana App", nullptr, nullptr);
    // TRint app`("Banana App", nullptr, nullptr);

    // Declare the compiled function to ROOT's interpreter so TButton can call it
    gInterpreter->Declare("extern \"C\" void plot_telescope(int det_id, const char *input_root_filename);");

    TList banana_list;

    // TBananaEditor editor("banana_editor");
    // editor.Init(nullptr, 800, 600, h, &banana_list, 0);

    CreateCanvasWithButtons();
    app.Run();

    return 0;
}
