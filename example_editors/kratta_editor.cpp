#include "TApplication.h"
#include "TButton.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH2D.h"
#include "TInterpreter.h"
#include "TPad.h"

#include "RQ_OBJECT.h"
#include "TBananaEditor.hh"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>

std::string INPUT_ROOT_FILENAME{"example_data/kratta_sum.root"};
std::string MATRIX_NAME_PATTERN{"KRATTA/kratta_pd0pd1_%02i"};
std::string OUTPUT_PATH{"."};

class ApplicationTerminator
{
    RQ_OBJECT("ApplicationTerminator")

  public:
    void Terminate()
    {
        if (gApplication)
        {
            gApplication->Terminate(0);
        }
    }
};

ApplicationTerminator gApplicationTerminator;

namespace
{
constexpr std::array<int, 16> kAllowedDetectorIds = {
    1, 2, 4, 5, 8, 9, 13, 17, 21, 22, 24, 25, 30, 33, 34, 38};

bool is_allowed_detector_id(int det_id)
{
    return std::find(kAllowedDetectorIds.begin(), kAllowedDetectorIds.end(), det_id) != kAllowedDetectorIds.end();
}

std::string matrix_name_for_id(const std::string &pattern, int det_id)
{
    return Form(pattern.c_str(), det_id);
}

void print_usage(const char *program)
{
    std::cerr << "Usage: " << program << " [input.root] [matrix-name-pattern] [output-directory]\n"
              << "\n"
              << "Defaults:\n"
              << "  input.root           " << INPUT_ROOT_FILENAME << "\n"
              << "  matrix-name-pattern  " << MATRIX_NAME_PATTERN << "\n"
              << "  output-directory     " << OUTPUT_PATH << "\n"
              << "\n"
              << "Example:\n"
              << "  " << program << " matrices.root folder/h_matrix_id%02i bananas\n";
}
} // namespace

extern "C" void plot_pd0pd1(int det_id, const char *input_root_filename, const char *matrix_name_pattern)
{
    if (!is_allowed_detector_id(det_id))
    {
        std::cerr << "Detector ID " << det_id << " is disabled for this editor\n";
        return;
    }

    TFile file_data(input_root_filename, "READ");
    if (file_data.IsZombie())
    {
        std::cerr << "Cannot open input ROOT file: " << input_root_filename << "\n";
        return;
    }

    std::string hname = matrix_name_for_id(matrix_name_pattern, det_id);

    // get histo
    TH2 *input_matrix = dynamic_cast<TH2 *>(file_data.Get(hname.c_str()));
    if (!input_matrix)
    {
        std::cerr << "Cannot find TH2 matrix '" << hname << "' in " << input_root_filename << "\n";
        return;
    }
    TH2 *matrix = dynamic_cast<TH2 *>(input_matrix->Clone(Form("matrix_id_%02d", det_id)));
    if (!matrix)
    {
        std::cerr << "Cannot clone TH2 matrix '" << hname << "'\n";
        return;
    }
    matrix->SetDirectory(0);
    file_data.Close();
    // get banana list

    TList *banana_list = new TList();

    TBananaEditor editor(Form("Banana editor KRATTA %i", det_id));

    std::string fname = OUTPUT_PATH + "/kratta_pd0pd1_" + std::to_string(det_id) + ".root";
    editor.SaveToFile(fname.c_str());
    editor.Init(nullptr, 1600, 800, matrix, banana_list, 0);
    delete matrix;
}

void CreateCanvasWithButtons()
{
    // Create a new canvas
    TCanvas *canvas = new TCanvas("KRATTA", "KRATTA array map", 600, 350);
    // Closing the selector window should end the ROOT application loop.
    canvas->Connect("Closed()", "ApplicationTerminator", &gApplicationTerminator, "Terminate()");

    // Divide the canvas into 64 pads (16x4) + a gap (16x2) + 64 pads (16x4)
    const int ncol = 8;
    const int nrow = 5;
    canvas->Divide(ncol, nrow);

    // Loop over the pads

    for (int x = 0; x < ncol; x++)
    {
        for (int y = 0; y < nrow; y++)
        {
            int button_pos = x + ncol * y;
            int det_id = x + ncol * y;
            // Set the pad's unique (x,y) position
            auto *pad = canvas->cd(button_pos + 1);
            pad->SetTopMargin(0);
            pad->SetBottomMargin(0);
            pad->SetLeftMargin(0);
            pad->SetRightMargin(0);
            // Create a button on the pad
            const bool enabled = is_allowed_detector_id(det_id);
            TButton *button = new TButton(
                Form("%i", det_id),
                enabled ? Form("plot_pd0pd1(%i,\"%s\",\"%s\")", det_id, INPUT_ROOT_FILENAME.c_str(),
                               MATRIX_NAME_PATTERN.c_str())
                        : "",
                0.0, 0.0, 1.0, 1.0);
            button->SetFillColor(TColor::GetColor("#7CB342")); // Light olive green
            if (!enabled)
            {
                button->SetFillColor(kBlack);
                button->SetTextColor(kGray + 2);
            }
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
    if (argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help"))
    {
        print_usage(argv[0]);
        return 0;
    }
    if (argc >= 2)
    {
        INPUT_ROOT_FILENAME = argv[1];
    }
    if (argc >= 3)
    {
        MATRIX_NAME_PATTERN = argv[2];
    }
    if (argc >= 4)
    {
        OUTPUT_PATH = argv[3];
    }

    TApplication app("app", nullptr, nullptr);

    // Declare the compiled function to ROOT's interpreter so TButton can call it
    gInterpreter->Declare(
        "extern \"C\" void plot_pd0pd1(int det_id, const char *input_root_filename, const char *matrix_name_pattern);");

    TList banana_list;

    // TBananaEditor editor("banana_editor");
    // editor.Init(nullptr, 800, 600, h, &banana_list, 0);

    CreateCanvasWithButtons();
    app.Run();

    return 0;
}
