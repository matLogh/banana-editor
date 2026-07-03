#include "TApplication.h"
#include "TButton.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH2D.h"
#include "TInterpreter.h"
#include "TPad.h"

#include "TBananaEditor.hh"

#include <iostream>
#include <string>

std::string INPUT_ROOT_FILENAME{"example_data/kratta_sum.root"};
std::string OUTPUT_PATH{"."};

extern "C" void plot_pd0pd1(int det_id, const char *input_root_filename)
{
    TFile file_data(input_root_filename, "READ");

    std::string hname = "KRATTA/kratta_pd0pd1_" + std::to_string(det_id);

    // get histo
    TH2 *matrix = (TH2 *)gDirectory->Get(hname.c_str());
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
    if (gApplication)
    {
        canvas->Connect("Closed()", "TApplication", gApplication, "Terminate()");
    }

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
            TButton *button =
                new TButton(Form("%i", det_id), Form("plot_pd0pd1(%i,\"%s\")", det_id, INPUT_ROOT_FILENAME.c_str()),
                            0.0, 0.0, 1.0, 1.0);
            button->SetFillColor(TColor::GetColor("#7CB342")); // Light olive green
            if (x == 3 || x == 7)
            {
                button->SetFillColor(kBlack); // Black for switched-off detectors
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

    if (argc == 2)
    {
        INPUT_ROOT_FILENAME = argv[1];
    }

    TApplication app("app", nullptr, nullptr);

    // Declare the compiled function to ROOT's interpreter so TButton can call it
    gInterpreter->Declare("extern \"C\" void plot_pd0pd1(int det_id, const char *input_root_filename);");

    TList banana_list;

    // TBananaEditor editor("banana_editor");
    // editor.Init(nullptr, 800, 600, h, &banana_list, 0);

    CreateCanvasWithButtons();
    app.Run();

    return 0;
}