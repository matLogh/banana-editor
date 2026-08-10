#include "TBananaEditor.hh"

#include <TCutG.h>

namespace
{
void WriteGraphAsCutG(const TGraph *graph)
{
    TCutG cut(graph->GetName(), graph->GetN());
    cut.SetTitle(graph->GetTitle());
    cut.SetLineColor(graph->GetLineColor());
    cut.SetLineStyle(graph->GetLineStyle());
    cut.SetLineWidth(graph->GetLineWidth());
    cut.SetFillColor(graph->GetFillColor());
    cut.SetFillStyle(graph->GetFillStyle());
    cut.SetMarkerColor(graph->GetMarkerColor());
    cut.SetMarkerStyle(graph->GetMarkerStyle());
    cut.SetMarkerSize(graph->GetMarkerSize());

    for (int i = 0; i < graph->GetN(); ++i)
    {
        double x = 0.0;
        double y = 0.0;
        graph->GetPoint(i, x, y);
        cut.SetPoint(i, x, y);
    }

    cut.Write();
}

void WriteBananaObject(TObject *obj)
{
    TGraph *graph = dynamic_cast<TGraph *>(obj);
    if (graph)
    {
        WriteGraphAsCutG(graph);
        return;
    }

    obj->Write();
}
} // namespace

ClassImpQ(TBananaEditor)

    TBananaEditor::TBananaEditor(const char *colName)
    : TObject(), fCollectionName(colName), fDefaultBananaNameEntry(nullptr)
{
    std::cout << "--------------------------\n";
    std::cout << "Keyboard Function Summary:\n"
              << "'d': Move histogram view to the right.\n"
              << "'a': Move histogram view to the left.\n"
              << "'r': Reset histogram view to original.\n"
              << "'w': Scale up the y-axis of the histogram.\n"
              << "'s': Scale down the y-axis of the histogram.\n"
              << "'e': Zoom in on the x-axis of the histogram.\n"
              << "'q': Zoom out on the x-axis of the histogram.\n"
              << "'x': Scale up the x-axis of the histogram.\n"
              << "'z': Scale down the x-axis of the histogram.\n"
              << "'m': Modify bananas if no current edit operation is ongoing.\n"
              << "'b': Add new banana if no current edit operation is ongoing.\n"
              << "'l': change log/lin scale for Z axis.\n";
};

void TBananaEditor::KeyInteract(Int_t event, Int_t px, Int_t /*py*/, TObject * /*selected*/)
{
    if (event != kKeyPress)
    {
        return;
    }

    TH1 *histo = nullptr;
    TIter next(gPad->GetListOfPrimitives());
    TObject *obj;
    while ((obj = next()))
    {
        histo = dynamic_cast<TH1 *>(obj);
        if (histo)
            break;
    }

    if (!histo)
    {
        return;
    }

    double xmax, xmin, ymin, ymax;
    switch (px)
    {
    case 'd': // MOVE LEFT
    {
        xmax = gPad->GetUxmax();
        xmin = gPad->GetUxmin();
        histo->GetXaxis()->SetRangeUser(xmin + (xmax - xmin) / 4, xmax + (xmax - xmin) / 4);
        break;
    }
    case 'a': // MOVE RIGHT
    {
        xmax = gPad->GetUxmax();
        xmin = gPad->GetUxmin();
        histo->GetXaxis()->SetRangeUser(xmin - (xmax - xmin) / 4, xmax - (xmax - xmin) / 4);
        break;
    }
    case 'r': // RESET
    {
        histo->GetXaxis()->UnZoom(); // SetRangeUser(0,maxX);
        histo->GetYaxis()->UnZoom(); // SetRangeUser(0,maxY);
        break;
    }
    case 'w': // SCALE UP
    {

        ymax = gPad->GetUymax();
        ymin = gPad->GetUymin();
        histo->GetYaxis()->SetRangeUser(ymin + (ymax - ymin) / 4, ymax + (ymax - ymin) / 4);
        break;
    }
    case 's': // SCALE DOWN
    {
        ymax = gPad->GetUymax();
        ymin = gPad->GetUymin();
        histo->GetYaxis()->SetRangeUser(ymin - (ymax - ymin) / 4, ymax - (ymax - ymin) / 4);
        break;
    }
    case 'e': // X axis  ZOOM
    {
        xmax = gPad->GetUxmax() * 0.95;
        xmin = gPad->GetUxmin() * 1.05;

        histo->GetXaxis()->SetRangeUser(xmin, xmax);
        break;
    }
    case 'q': // X axis  UNZOOM
    {
        xmax = gPad->GetUxmax() * 1.05;
        xmin = gPad->GetUxmin() * 0.95;

        histo->GetXaxis()->SetRangeUser(xmin, xmax);
        break;
    }
    case 'x': // SCALE UP
    {
        ymax = gPad->GetUymax() * 0.95;
        ymin = gPad->GetUymin() * 1.05;
        histo->GetYaxis()->SetRangeUser(ymin, ymax);
        break;
    }
    case 'z': // SCALE DOWN
    {
        ymax = gPad->GetUymax() * 1.05;
        ymin = gPad->GetUymin() * 0.95;

        histo->GetYaxis()->SetRangeUser(ymin, ymax);
        break;
    }
    case 'm': // modify bananas
    {

        if (fBananaEditStatus == kNothing)
        {
            DoEditBanana();
        }
        break;
    }
    case 'b': // add new banana
    {

        if (fBananaEditStatus == kNothing)
        {
            AddNewBanana();
        }
        break;
    }
    case 'l': // add new banana
    {
        gPad->SetLogz(!gPad->GetLogz());
        break;
    }
    case '\n': // ok action
    {

        if (fBananaEditStatus == kAddBanana2D || fBananaEditStatus == kEditBanana2D ||
            fBananaEditStatus == kAddBanana1D || fBananaEditStatus == kEditBanana1D)
        {
            OkAction();
        }
        break;
    }
    default:
        return;
    };

    gPad->Modified();
    gPad->Update();
}

void TBananaEditor::DrawDefaultCanvas()
{
    fCanvas->SetFillColor(TColor::GetColor("#F5F5F0")); // Warm off-white to complement green palette

    // Set a modern color palette that matches the professional theme
    gStyle->SetPalette(kAvocado); // Green-based palette

    if (fHist->InheritsFrom(TH2::Class()))
    {
        TH2 *h = (TH2 *)fHist;
        fCanvas->cd();
        h->GetXaxis()->SetRangeUser(50, h->GetXaxis()->GetXmax());
        h->GetYaxis()->SetRangeUser(50, h->GetYaxis()->GetXmax());
        h->Draw("COLZ0");
    }
    else
    {
        fCanvas->cd();
        fHist->Draw();
    }
    fHist->SetStats(kFALSE);
    fCanvas->Update();
    fCanvas->GetFrame()->SetFillColor(TColor::GetColor("#FAFAF5")); // Light cream frame
    fCanvas->GetFrame()->SetBorderSize(6);                          // Thinner, modern border
    fCanvas->Modified();
    fCanvas->Update();
    // this thing disables movement of a histogram frame once clicked and dragged
    gPad->GetFrame()->SetBit(TBox::kCannotMove);
    DrawAllBananas();
}

void TBananaEditor::SaveToFile(const std::string savefile)
{
    fSaveFileName = savefile; // Store filename for saving on exit
    fBananaList = new TList();

    // Load existing bananas if file exists
    if (!gSystem->AccessPathName(savefile.c_str()))
    {
        TFile *inputFile = TFile::Open(savefile.c_str(), "READ");
        if (inputFile && inputFile->IsOpen())
        {
            fBananaList->SetName("fBananaList");
            TIter next(inputFile->GetListOfKeys());
            TKey *key;
            while ((key = (TKey *)next()))
            {
                TObject *obj = key->ReadObj();
                if (obj->InheritsFrom(TGraph::Class()))
                {
                    fBananaList->Add(obj);
                }
            }
            inputFile->Close();
            delete inputFile;
        }
    }
    fBananaListTemp = (TList *)fBananaList->Clone("fBananaListTemp");
}

void TBananaEditor::SetDefaultBananaName(const char *name)
{
    SetDefaultBananaName(name ? std::string(name) : std::string());
}

void TBananaEditor::SetDefaultBananaName(const std::string &name)
{
    fDefaultBananaNamePrefix = name;
    if (fDefaultBananaNameEntry)
    {
        fDefaultBananaNameEntry->SetText(fDefaultBananaNamePrefix.c_str());
    }
}

void TBananaEditor::CancelAction()
{
    if ((fBananaEditStatus == kAddBanana2D) || (fBananaEditStatus == kEditBanana2D))
    {
        fCanvas->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this,
                            "EditBanana(Int_t,Int_t,Int_t,TObject*)");
        fCanvas->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this,
                            "Make2DBanana(Int_t,Int_t,Int_t,TObject*)");
    }
    if ((fBananaEditStatus == kAddBanana1D) || (fBananaEditStatus == kEditBanana1D))
    {
        fCanvas->Disconnect("Highlighted(TVirtualPad*, TObject*, Int_t, Int_t)", this,
                            "ListedForHighlightPoint(TVirtualPad*, TObject*, Int_t, Int_t)");
        fCanvas->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this,
                            "EditBanana(Int_t,Int_t,Int_t,TObject*)");
    }

    // fBananaListTemp->Delete(); // delete all list members
    delete fBananaListTemp;
    fBananaListTemp = (TList *)fBananaList->Clone("fBananaListTemp");

    fBananaEditStatus = kNothing;
    DrawAllBananas(); // refresh all bananas
    fMain->HideFrame(hframe0);

    fBtnNewBanana->SetState(kButtonUp);
    fBtnEditBanana->SetState(kButtonUp);
    // if (IsEdited())
    // fBtnSave->SetState(kButtonUp);
    fBtnExit->SetState(kButtonUp);
}

void TBananaEditor::OkAction()
{
    // disconnect slots, check if >=3 points are present and add point to close the loop if necessary
    if ((fBananaEditStatus == kAddBanana2D) || (fBananaEditStatus == kEditBanana2D))
    {
        // we need to make sure that all bananas has at least 3 points
        TIter _next(fBananaListTemp);
        while (TObject *obj = _next())
        {
            fGraph = (TGraph *)obj;
            if (fGraph->GetN() < 3)
            {
                cout << "***TBananaEditor::OkAction()" << endl << "Not enough points for OK action!" << endl;
                int retval;

                fMain->Disconnect("CloseWindow()");
                fMain->Connect("CloseWindow()", "TBananaEditor", this, "TryToClose()");
                new TGMsgBox(gClient->GetRoot(), fMain, fMain->GetWindowName(), "At least 3 points are required!",
                             kMBIconStop, kMBClose, &retval);
                fMain->Disconnect("CloseWindow()");
                fMain->Connect("CloseWindow()", "TBananaEditor", this, "DoExit()");
                // exit without doint the OK action
                return;
            }
        }
        // disconnect slots
        fCanvas->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this,
                            "EditBanana(Int_t,Int_t,Int_t,TObject*)");
        fCanvas->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this,
                            "Make2DBanana(Int_t,Int_t,Int_t,TObject*)");
        fCanvas->SetEditable(true);

        // if necessary, add last point at the same coordinates as the first one to enclose the region
        double x_first, y_first, x_last, y_last;
        TIter next(fBananaListTemp);
        while (TObject *obj = next())
        {
            fGraph = (TGraph *)obj;
            fGraph->GetPoint(0, x_first, y_first);
            fGraph->GetPoint(fGraph->GetN() - 1, x_last, y_last);
            if ((x_first != x_last) || (y_first != y_last))
            {
                fGraph->SetEditable(true);
                // fGraph->AddPoint(x_first,y_first); // only for ROOT>6.24
                fGraph->SetPoint(fGraph->GetN(), x_first, y_first);
            }
            fGraph->SetEditable(false);
        }
    }
    // disconnect slots
    if ((fBananaEditStatus == kAddBanana1D) || (fBananaEditStatus == kEditBanana1D))
    {
        fCanvas->Disconnect("Highlighted(TVirtualPad*, TObject*, Int_t, Int_t)", this,
                            "ListedForHighlightPoint(TVirtualPad*, TObject*, Int_t, Int_t)");
        fCanvas->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this,
                            "EditBanana(Int_t,Int_t,Int_t,TObject*)");
    }

    fGraph = 0;
    fBananaEditStatus = kNothing;
    fMain->HideFrame(hframe0);

    // replace fBananaList with the temporary one
    // fBananaList->Delete(); // delete all list members
    // delete fBananaList;
    // fBananaList = (TList *)fBananaListTemp->Clone("fBananaList");

    // check for new bananas

    // check for deleted
    // if (fBananaListTemp->GetSize() != fBananaList->GetSize())
    // {
    //   TIter next(fBananaList);
    //   while (TObject *obj = next())
    //   {
    //     if (fBananaListTemp->FindObject(obj) != 0)
    //       continue;
    //     delete obj;
    //     fBananaList->Remove(obj);
    //   }
    // }

    fBtnNewBanana->SetState(kButtonUp);
    fBtnEditBanana->SetState(kButtonUp);
    // if (IsEdited())
    //     fBtnSave->SetState(kButtonUp);
    fBtnExit->SetState(kButtonUp);

    fBananaList->Clear("nodelete");
    TIter next_temp(fBananaListTemp);
    while (TObject *obj_temp = next_temp())
    {
        fBananaList->Add(obj_temp);
    }

    DrawAllBananas(); // refresh all bananas
}

void TBananaEditor::ListedForHighlightPoint(TVirtualPad * /*pad*/, TObject *selected, Int_t x, Int_t /*y*/)
{
    // (void *)pad;
    if (x != 1 && x != 4 && x != 7)
        selectedPoint = -1;
    else
        selectedPoint = x;
    fGraph = (TGraph *)selected;
}

void TBananaEditor::DoEditBanana()
{
    fMain->ShowFrame(hframe0);
    fBtnNewBanana->SetState(kButtonDisabled);
    fBtnEditBanana->SetState(kButtonDisabled);
    // fBtnSave->SetState(kButtonDisabled);
    fBtnExit->SetState(kButtonDisabled);

    if (fHist->InheritsFrom(TH2::Class()))
    {
        fBananaEditStatus = kEditBanana2D;
    }
    else if (fHist->InheritsFrom(TH1::Class()))
    {
        cout << "inherits from th1 " << endl;
        fBananaEditStatus = kEditBanana1D;
        fCanvas->Connect("Highlighted(TVirtualPad*, TObject*, Int_t, Int_t)", "TBananaEditor", this,
                         "ListedForHighlightPoint(TVirtualPad*, TObject*, Int_t, Int_t)");
    }
    fCanvas->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TBananaEditor", this,
                     "EditBanana(Int_t,Int_t,Int_t,TObject*)");

    DrawAllBananas();
}

void TBananaEditor::MoveGraphPoints(TGraph *g, Double_t dx, Double_t dy)
{
    Double_t x = 0, y = 0;
    for (Int_t i = 0; i < g->GetN(); i++)
    {
        g->GetPoint(i, x, y);
        x += dx;
        y += dy;
        g->SetPoint(i, x, y);
    }
}

void TBananaEditor::EditBanana(Int_t event, Int_t px, Int_t /*py*/, TObject *selected)
{
    // if(event == 61) fGraph->SetEditable(false);

    if (fBananaEditStatus == kNothing)
    {
        fCanvas->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this,
                            "EditBanana(Int_t,Int_t,Int_t,TObject*)");
        return;
    }

    if (fBananaEditStatus == kEditBanana2D)
    {
        // nothing to do, all is already handled by the default ROOT code
    }
    if (fBananaEditStatus == kEditBanana1D)
    {
        if (fHist->InheritsFrom(TH1::Class()))
        {
            // move points in case axis range change
            if (selected->InheritsFrom(TAxis::Class()))
            {
                DrawBanana(fGraph);
                fCanvas->SetCursor(kHand);
                return;
            }
            // moving the banana-gate
            if (event != 21)
                return;
            if (selected->InheritsFrom(TGraph::Class()))
            {
                if (selectedPoint == -1)
                    return;
                fGraph = (TGraph *)selected;
                fGraph->SetEditable(true);
                // get histogram coordinates
                double xmouse = gPad->PadtoX(gPad->AbsPixeltoX(px));
                // double ymouse = gPad->PadtoX(gPad->AbsPixeltoX(py)); // unused
                double xpoint, ypoint; // double xpoint = fGraph->GetPointX(selectedPoint);
                fGraph->GetPoint(selectedPoint, xpoint, ypoint);
                double dx = xmouse - xpoint;
                double x;
                double y;
                if (selectedPoint == 1)
                {
                    fGraph->GetPoint(0, x, y);
                    fGraph->SetPoint(0, x + dx, y);
                    fGraph->GetPoint(1, x, y);
                    fGraph->SetPoint(1, x + dx, y);
                    fGraph->GetPoint(2, x, y);
                    fGraph->SetPoint(2, x + dx, y);

                    fGraph->GetPoint(6, x, y);
                    fGraph->SetPoint(6, x + dx / 2., y);
                    fGraph->GetPoint(7, x, y);
                    fGraph->SetPoint(7, x + dx / 2., y);
                    fGraph->GetPoint(8, x, y);
                    fGraph->SetPoint(8, x + dx / 2., y);
                }
                if (selectedPoint == 7)
                {
                    MoveGraphPoints(fGraph, dx, 0);
                    // fGraph->MovePoints(dx,0); //since ROOT>6.2x
                }
                if (selectedPoint == 4)
                {
                    fGraph->GetPoint(3, x, y);
                    fGraph->SetPoint(3, x + dx, y);
                    fGraph->GetPoint(4, x, y);
                    fGraph->SetPoint(4, x + dx, y);
                    fGraph->GetPoint(5, x, y);
                    fGraph->SetPoint(5, x + dx, y);

                    fGraph->GetPoint(6, x, y);
                    fGraph->SetPoint(6, x + dx / 2., y);
                    fGraph->GetPoint(7, x, y);
                    fGraph->SetPoint(7, x + dx / 2., y);
                    fGraph->GetPoint(8, x, y);
                    fGraph->SetPoint(8, x + dx / 2., y);
                }
                fCanvas->cd();
                fGraph->Draw("PF");
                fCanvas->Modified();
                fCanvas->Update();
                fGraph->SetEditable(false);
            }
        }
    }
}

void TBananaEditor::Make2DBanana(Int_t event, Int_t px, Int_t py, TObject * /*selected*/)
{
    // if((selected->InheritsFrom(TH2::Class())) || (selected->InheritsFrom(TGraph::Class())))
    {
        fCanvas->SetCursor(kCross);
        // if(event == 61) return;
        if (event == 11) // 11= single left click
        {
            gPad->GetCanvas()->FeedbackMode(kTRUE);
            // fGraph->AddPoint(gPad->PadtoX(gPad->AbsPixeltoX(px)), gPad->PadtoY(gPad->AbsPixeltoY(py))); //>ROOT 6.2?
            fGraph->SetPoint(fGraph->GetN(), gPad->PadtoX(gPad->AbsPixeltoX(px)), gPad->PadtoY(gPad->AbsPixeltoY(py)));
            fCanvas->cd();
            fCanvas->SetEditable(true);
            fGraph->Draw("LP");
            fCanvas->Modified();
            fCanvas->Update();
            fCanvas->SetEditable(false);
            return;
        }
        if (event == 61) // double click
        {
            OkAction();
        }
    }
}

void TBananaEditor::DrawAllBananas()
{
    TIter next(fBananaListTemp);
    while (TObject *obj = next())
    {
        TGraph *gr = (TGraph *)obj;
        DrawBanana(gr);
    }
}

void TBananaEditor::DrawBanana(TGraph *gr)
{
    if (!gr)
        return;
    if (fHist->InheritsFrom(TH2::Class()))
    {
        gr->SetLineWidth(3);
        gr->SetMarkerStyle(20);
        gr->SetMarkerSize(1.);
        if (fBananaEditStatus == kEditBanana2D)
        {
            gr->SetHighlight(true);
            gr->SetEditable(true);
            gr->SetMarkerColor(kAzure); // blue for editing
        }
        else if (fBananaEditStatus == kAddBanana2D)
        {
            gr->SetHighlight(true);
            gr->SetEditable(true);
            gr->SetMarkerColor(kViolet + 1); // Forest green for adding
            gr->SetLineColor(kMagenta + 2);
        }
        else
        {
            gr->SetHighlight(false);
            gr->SetEditable(false);
            gr->SetMarkerColor(kRed + 1); // Red-orange for display
            gr->SetLineColor(kYellow + 2);
        }
        fCanvas->cd();
        gr->Draw("LP");
        fCanvas->Modified();
        fCanvas->Update();
    }
    else if (fHist->InheritsFrom(TH1::Class()))
    {
        double x_min, x_max;
        double y_min, y_max;
        gPad->GetRangeAxis(x_min, y_min, x_max, y_max);
        double y_mid = (y_max + y_min) / 2.;
        double x_point, y_point;
        double ydown = y_min - abs(y_max);
        double yup = y_max + abs(y_max);

        gr->GetPoint(0, x_point, y_point);
        gr->SetPoint(0, x_point, ydown); // 0
        gr->SetPoint(1, x_point, y_mid); // 0
        gr->SetPoint(2, x_point, yup);   // 0

        gr->GetPoint(3, x_point, y_point);
        gr->SetPoint(3, x_point, yup);   // 3
        gr->SetPoint(4, x_point, y_mid); // 4 ! important
        gr->SetPoint(5, x_point, ydown); // 5

        gr->GetPoint(6, x_point, y_point);
        gr->SetPoint(6, x_point, ydown); // 6
        gr->SetPoint(7, x_point, y_mid); // 7 ! important
        gr->SetPoint(8, x_point, ydown); // 8

        gr->SetFillStyle(3001);

        if (fBananaEditStatus == kEditBanana1D)
        {
            gr->SetHighlight(true);
            gr->SetEditable(false);
            gr->SetMarkerColor(kAzure); // Professional blue for editing
            // fGraph->SetFillColor(kAzure);
        }
        else
        {
            gr->SetHighlight(false);
            gr->SetEditable(false);
            gr->SetMarkerColor(kOrange + 8); // Red-orange for display
            gr->SetFillColor(kOrange + 8);
        }
        fCanvas->cd();
        gr->Draw("PF");
        fCanvas->Modified();
        fCanvas->Update();
    }
}

void TBananaEditor::AddNewBanana()
{
    fGraph = new TGraph();
    fGraph->SetLineWidth(3);
    fGraph->SetMarkerStyle(20);
    fGraph->SetMarkerSize(1.);
    // set name for tgraph-banana
    char name[256];
    snprintf(name, sizeof(name), "%s_%i", GetDefaultBananaNamePrefix().c_str(), GetNextFreeBananaNumber());
    fGraph->SetName(name);
    cout << "new banana name: " << fGraph->GetName() << endl;
    // add banana to the temporary banana list
    fBananaListTemp->Add(fGraph);

    if (fHist->InheritsFrom(TH2::Class()))
    {
        fMain->ShowFrame(hframe0);
        fBtnNewBanana->SetState(kButtonDisabled);
        fBtnEditBanana->SetState(kButtonDisabled);
        // fBtnSave->SetState(kButtonDisabled);
        fBtnExit->SetState(kButtonDisabled);

        fBananaEditStatus = kAddBanana2D;
        fGraph->SetMarkerColor(kViolet + 1); // Forest green for adding
        fGraph->SetLineColor(kMagenta + 2);
        // fGraph->SetMarkerColor(kGreen + 2); // Forest green for new banana
        fCanvas->SetEditable(false);
        fCanvas->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TBananaEditor", this,
                         "Make2DBanana(Int_t,Int_t,Int_t,TObject*)");
    }
    else if (fHist->InheritsFrom(TH1::Class()))
    {
        fBananaEditStatus = kAddBanana1D;

        double x_min, x_max;
        double y_min, y_max;
        gPad->GetRangeAxis(x_min, y_min, x_max, y_max);
        double x_mid = (x_max + x_min) / 2.;
        double y_mid = (y_max + y_min) / 2.;

        double xleft = x_mid - abs(0.1 * (x_max - x_min));
        double xright = x_mid + abs(0.1 * (x_max - x_min));
        double ydown = y_min - abs(y_min);
        double yup = y_max + abs(y_max);

        int n = fGraph->GetN();

        fGraph->SetPoint(n++, xleft, ydown);  // 0
        fGraph->SetPoint(n++, xleft, y_mid);  // 1 ! important
        fGraph->SetPoint(n++, xleft, yup);    // 2
        fGraph->SetPoint(n++, xright, yup);   // 3
        fGraph->SetPoint(n++, xright, y_mid); // 4 ! important
        fGraph->SetPoint(n++, xright, ydown); // 5
        fGraph->SetPoint(n++, x_mid, ydown);  // 6
        fGraph->SetPoint(n++, x_mid, y_mid);  // 7 ! important
        fGraph->SetPoint(n++, x_mid, ydown);  // 8
        fGraph->SetMarkerColor(kOrange + 8);  // Red-orange for display
        fGraph->SetHighlight(false);
        fGraph->SetEditable(false);
        fGraph->SetFillStyle(3001);
        fGraph->SetFillColor(kOrange + 8);

        fCanvas->cd();
        fGraph->Draw("PF");
        fCanvas->Modified();
        fCanvas->Update();

        selectedPoint = -1;
        DoEditBanana();
    }
}

// check if some modifications has been done since last SAVE was clicked
bool TBananaEditor::IsEdited()
{
    if (fBananaList->GetSize() != fBananaListTemp->GetSize())
        return true;
    TIter next(fBananaList);
    while (TObject *obj = next())
    {
        TGraph *obj1 = (TGraph *)obj;
        TGraph *obj2 = (TGraph *)fBananaListTemp->FindObject(obj->GetName());
        // is same graph in original list?
        if (!obj2)
            return true;
        int n1 = obj1->GetN();
        int n2 = obj2->GetN();
        // are they the same length?
        if (n1 != n2)
            return true;
        double *x1, *x2, *y1, *y2;
        x1 = obj1->GetX();
        y1 = obj1->GetY();
        x2 = obj2->GetX();
        y2 = obj2->GetY();
        // are the points identical?
        for (int i = 0; i < n1; i++)
        {
            if ((x1[i] != x2[i]) || (y1[i] != y2[i]))
                return true;
        }
    }
    return false;
}

void TBananaEditor::DoSave()
{
    fMain->Disconnect("CloseWindow()");
    // fMain->Connect("CloseWindow()", "TBananaEditor", this, "TryToClose()");
    // int retval;
    // new TGMsgBox(
    //     gClient->GetRoot(), fMain, fMain->GetWindowName(),
    //     "Saving will send bananas to the server, this action will force-clear histogram set! Do you want to
    //     proceed?", kMBIconQuestion, kMBYes | kMBNo, &retval);
    // fMain->Disconnect("CloseWindow()");
    // fMain->Connect("CloseWindow()", "TBananaEditor", this, "DoExit()");
    // if (retval == kMBYes)
    // {
    // char name[100];
    // sprintf(name, "%s_banList", fHist->GetName());
    // TList *returnList = (TList *)fBananaList->Clone(name);
    // NewBananasFromEditor(returnList);
    // Emit("NewBananasFromEditor(TList*)", returnList);
    // fBtnSave->SetState(kButtonDisabled);
    // }

    TFile *outputFile = TFile::Open(fSaveFileName.c_str(), "RECREATE");
    if (outputFile && outputFile->IsOpen())
    {
        TIter next(fBananaListTemp);
        TObject *obj;
        while ((obj = next()))
        {
            std::cout << "Saving banana: " << obj->GetName() << std::endl;
            outputFile->cd();
            WriteBananaObject(obj);
        }

        outputFile->Close();
        delete outputFile;
        std::cout << "Bananas saved to: " << fSaveFileName << std::endl;
    }
    else
    {
        std::cerr << "Error: Could not save bananas to " << fSaveFileName << std::endl;
    }

    fSaveFileName = ""; // Clear the filename to prevent saving on exit
}

// void TBananaEditor::NewBananasFromEditor(TList *bananaList)
// {

// }

void TBananaEditor::DoExit()
{
    int retval;
    bool isEdited = IsEdited();

    if (isEdited)
    {
        fMain->Disconnect("CloseWindow()");
        fMain->Connect("CloseWindow()", "TBananaEditor", this, "TryToClose()");
        if (isEdited)
            new TGMsgBox(gClient->GetRoot(), fMain, fMain->GetWindowName(),
                         "Some modifications were NOT SAVED. Save before exiting?", kMBIconExclamation,
                         kMBYes | kMBNo | kMBCancel, &retval);
        fMain->Disconnect("CloseWindow()");
        // fMain->Connect("CloseWindow()", "TBananaEditor", this, "DoExit()");
        if (retval == kMBYes)
        {
            DoSave();
            Emit("ClosingBananaEditor(Int_t)", fID);
            CloseWindow();
        }
        else if (retval == kMBNo)
        {
            Emit("ClosingBananaEditor(Int_t)", fID);
            CloseWindow();
        }
    }
    else
    {
        Emit("ClosingBananaEditor(Int_t)", fID);
        CloseWindow();
    }
}

void TBananaEditor::TryToClose()
{
    cout
        << "***TBananaEditor::TryToClose()" << endl
        << "Cannot close the window now! Possible reasons:\n   1)A message window is opened \n   2)Window is in editor "
           "mode"
        << endl;
}

void TBananaEditor::CloseWindow()
{
    // Save confirmed bananas (fBananaList) to file
    // Note: fBananaList only contains confirmed changes (updated via OkAction)
    // Unconfirmed changes in fBananaListTemp are NOT saved
    if (!fSaveFileName.empty())
    {
        TFile *outputFile = TFile::Open(fSaveFileName.c_str(), "RECREATE");
        if (outputFile && outputFile->IsOpen())
        {
            TIter next(fBananaList);
            TObject *obj;
            while ((obj = next()))
            {
                outputFile->cd();
                WriteBananaObject(obj);
            }
            outputFile->Close();
            delete outputFile;
            std::cout << "Bananas saved to: " << fSaveFileName << std::endl;
        }
        else
        {
            std::cerr << "Error: Could not save bananas to " << fSaveFileName << std::endl;
        }
    }

    Emit("ClosingBananaWindow(TBananaEditor*)", this);
    fMain->CloseWindow();

    // Cleanup
    if (fBananaListTemp)
    {
        delete fBananaListTemp;
        fBananaListTemp = nullptr;
    }
}

void TBananaEditor::SetStatusText(const char *txt, Int_t pi)
{
    // Set text in status bar.
    fStatusBar->SetText(txt, pi);
}

void TBananaEditor::EventInfo(Int_t event, Int_t px, Int_t py, TObject *selected)
{
    //  Writes the event status in the status bar parts
    const char *text0, *text1, *text3;
    char text2[50];
    text0 = selected->GetTitle();
    SetStatusText(text0, 0);
    text1 = selected->GetName();
    SetStatusText(text1, 1);
    if (event == kKeyPress)
        sprintf(text2, "%c", (char)px);
    else
        sprintf(text2, "%d,%d", px, py);
    SetStatusText(text2, 2);
    text3 = selected->GetObjectInfo(px, py);
    SetStatusText(text3, 3);
}

void TBananaEditor::AdjustContextMenu()
{
    TClass *cl;
    TList *ml;
    TClassMenuItem *mi;
    /*
    //TH2F
       cl = gROOT->GetClass("TH2F");
       cl->MakeCustomMenuList();
       ml = cl->GetMenuList();
       for(int i=0;i<16;i++) {mi = (TClassMenuItem*)ml->At(1); delete mi;}
       mi = (TClassMenuItem*)ml->At(4); delete mi;
       for(int i=0;i<17;i++) {mi = (TClassMenuItem*)ml->At(6); delete mi;}
       mi = new TClassMenuItem(TClassMenuItem::kPopupSeparator,cl);
       ml->AddLast(mi);
       mi = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,TBananaEditor::Class(), "Unzoom
    both","ContextUnZoomBoth",this,"TObject*",2); ml->AddLast(mi);
    //TH2D
       cl = gROOT->GetClass("TH2D");
       cl->MakeCustomMenuList();
       ml = cl->GetMenuList();
       for(int i=0;i<16;i++) {mi = (TClassMenuItem*)ml->At(1); delete mi;}
       mi = (TClassMenuItem*)ml->At(4); delete mi;
       for(int i=0;i<17;i++) {mi = (TClassMenuItem*)ml->At(6); delete mi;}
       mi = new TClassMenuItem(TClassMenuItem::kPopupSeparator,cl);
       ml->AddLast(mi);
       mi = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,TBananaEditor::Class(), "Unzoom
    both","ContextUnZoomBoth",this,"TObject*",2);
    //TAxis
       cl = gROOT->GetClass("TAxis");
       cl->MakeCustomMenuList();
       ml = cl->GetMenuList();
       for(int i=0;i<7;i++) {mi = (TClassMenuItem*)ml->At(1); delete mi;}
       for(int i=0;i<3;i++) {mi = (TClassMenuItem*)ml->At(3); delete mi;}
       for(int i=0;i<21;i++) {mi = (TClassMenuItem*)ml->At(5); delete mi;}
    //TPaletteAxis
       cl = gROOT->GetClass("TPaletteAxis");
       cl->MakeCustomMenuList();
       ml = cl->GetMenuList();
       for(int i=0;i<8;i++) {mi = (TClassMenuItem*)ml->At(1); delete mi;}
       for(int i=0;i<17;i++) {mi = (TClassMenuItem*)ml->At(2); delete mi;}
    //Tcanvas
       cl = gROOT->GetClass("TCanvas");
       cl->MakeCustomMenuList();
       ml = cl->GetMenuList();
       for(int i=0;i<1;i++) {mi = (TClassMenuItem*)ml->At(1); delete mi;}
       for(int i=0;i<2;i++) {mi = (TClassMenuItem*)ml->At(2); delete mi;}
       for(int i=0;i<5;i++) {mi = (TClassMenuItem*)ml->At(3); delete mi;}
       for(int i=0;i<5;i++) {mi = (TClassMenuItem*)ml->At(4); delete mi;}
       for(int i=0;i<11;i++) {mi = (TClassMenuItem*)ml->At(9); delete mi;}
    //TPaveText
       cl = gROOT->GetClass("TPaveText");
       cl->MakeCustomMenuList();
       ml = cl->GetMenuList();
       ml->Delete();
    //TPaveStats
       cl = gROOT->GetClass("TPaveStats");
       cl->MakeCustomMenuList();
       ml = cl->GetMenuList();
       ml->Delete();
    //TFrame
       cl = gROOT->GetClass("TFrame");
       cl->MakeCustomMenuList();
       ml = cl->GetMenuList();
       ml->Delete();
       */
    // TGraph
    cl = gROOT->GetClass("TGraph");
    cl->SetContextMenuTitle("Pseudo banana");
    cl->MakeCustomMenuList();
    ml = cl->GetMenuList();
    ml->Delete();

    mi = new TClassMenuItem(TClassMenuItem::kPopupUserFunction, TBananaEditor::Class(), "Insert point",
                            "ConetextInsertPoint", this, "TObject*", 2);
    ml->AddLast(mi);
    mi = new TClassMenuItem(TClassMenuItem::kPopupUserFunction, TBananaEditor::Class(), "Remove point",
                            "ContextRemovePoint", this, "TObject*", 2);
    ml->AddLast(mi);
    mi = new TClassMenuItem(TClassMenuItem::kPopupSeparator, cl);
    ml->AddLast(mi);
    mi = new TClassMenuItem(TClassMenuItem::kPopupUserFunction, TBananaEditor::Class(), "Rename banana",
                            "ContextRenameBanana", this, "TObject*", 2);
    ml->AddLast(mi);
    mi = new TClassMenuItem(TClassMenuItem::kPopupUserFunction, TBananaEditor::Class(), "Clone banana",
                            "ContextCloneBanana", this, "TObject*", 2);
    ml->AddLast(mi);
    mi = new TClassMenuItem(TClassMenuItem::kPopupSeparator, cl);
    ml->AddLast(mi);
    mi = new TClassMenuItem(TClassMenuItem::kPopupUserFunction, TBananaEditor::Class(), "Delete banana",
                            "ContextDeleteBanana", this, "TObject*", 2);
    ml->AddLast(mi);
}

void TBananaEditor::ContextInsertPoint(TObject *obj)
{
    if (obj->InheritsFrom(TGraph::Class()))
    {
        if (fBananaEditStatus == kEditBanana2D)
        {
            TGraph *gr = (TGraph *)obj;
            gr->InsertPoint();
        }
    }
}

void TBananaEditor::ContextRemovePoint(TObject *obj)
{
    if (obj->InheritsFrom(TGraph::Class()))
    {
        if (fBananaEditStatus == kEditBanana2D)
        {
            TGraph *gr = (TGraph *)obj;
            gr->RemovePoint();
        }
    }
}

void TBananaEditor::ContextDeleteBanana(TObject *obj)
{
    if (obj->InheritsFrom(TGraph::Class()))
    {
        if ((fBananaEditStatus == kEditBanana2D) || (fBananaEditStatus == kEditBanana1D))
        {
            TGraph *gr = (TGraph *)obj;
            // remove from canvas primitives and temporary banana list
            fBananaListTemp->Remove(gr);
            TList *l = fCanvas->GetListOfPrimitives();
            l->Remove(gr);
            delete gr;
        }
    }
}

void TBananaEditor::ContextUnZoomBoth(TObject *obj)
{
    if (obj->InheritsFrom(TH2::Class()))
    {
        TH2 *mat = (TH2 *)obj;
        mat->GetXaxis()->UnZoom();
        mat->GetYaxis()->UnZoom();
    }
}

void TBananaEditor::ContextRenameBanana(TObject *obj)
{
    if (obj->InheritsFrom(TGraph::Class()))
    {
        TGraph *gr = (TGraph *)obj;
        // char    name[100];

        fMain->Disconnect("CloseWindow()");
        fMain->Connect("CloseWindow()", "TBananaEditor", this, "TryToClose()");

        int retval;
        char newName[100];
        new TGInputDialog(0, fMain, "Type new name for this banana", gr->GetName(), newName, kVerticalFrame);
        if (strlen(newName) < 1)
        {
            new TGMsgBox(gClient->GetRoot(), fMain, fMain->GetWindowName(), "Invalid name!", kMBIconStop, kMBClose,
                         &retval);
            cout << "***TBananaEditor::ContextRenameBanana()"
                 << "\tInvalid name!" << endl;
            fMain->Disconnect("CloseWindow()");
            fMain->Connect("CloseWindow()", "TBananaEditor", this, "TryToClose()");
            return;
        }
        if (fBananaList->FindObject(newName) || fBananaListTemp->FindObject(newName))
        {
            new TGMsgBox(gClient->GetRoot(), fMain, fMain->GetWindowName(), "TGraph with this name already exists",
                         kMBIconStop, kMBClose, &retval);
            cout << "***TBananaEditor::ContextRenameBanana()"
                 << "\tInvalid name!" << endl;
            fMain->Disconnect("CloseWindow()");
            fMain->Connect("CloseWindow()", "TBananaEditor", this, "TryToClose()");
            return;
        }

        gr->SetName(newName);

        fMain->Disconnect("CloseWindow()");
        fMain->Connect("CloseWindow()", "TBananaEditor", this, "TryToClose()");
    }
}

void TBananaEditor::ContextCloneBanana(TObject *obj)
{
    if (obj->InheritsFrom(TGraph::Class()))
    {
        TGraph *gr = (TGraph *)obj;
        char name[256];
        snprintf(name, sizeof(name), "%s_%i", GetDefaultBananaNamePrefix().c_str(), GetNextFreeBananaNumber());
        fGraph = (TGraph *)gr->Clone(name);
        fBananaListTemp->Add(fGraph);
        if (fBananaEditStatus == kEditBanana2D)
        {
            // now calculate in which direction to move the banana
            double xmin = fHist->GetXaxis()->GetXmin();
            double xmax = fHist->GetXaxis()->GetXmax();
            double ymin = fHist->GetYaxis()->GetXmin();
            double ymax = fHist->GetYaxis()->GetXmax();
            // double* x = gr->GetX();
            // double avg=0;
            // for(int i=0;i<gr->GetN();i++) avg+=x[i];
            // avg = avg/(double)gr->GetN();
            MoveGraphPoints(fGraph, (xmax - xmin) * 0.1, (ymax - ymin) * 0.1);
            // fGraph->MovePoints((xmax-xmin)*0.1,(ymax-ymin)*0.1);
            fCanvas->cd();
            fGraph->Draw("LP");
        }
        else if (fBananaEditStatus == kEditBanana1D)
        {
            double xmin = fHist->GetXaxis()->GetXmin();
            double xmax = fHist->GetXaxis()->GetXmax();
            MoveGraphPoints(fGraph, (xmax - xmin) * 0.1, 0);
            // fGraph->MovePoints((xmax-xmin)*0.1,0);
            fCanvas->cd();
            fGraph->Draw("PF");
        }
        fCanvas->Update();
        fCanvas->Modified();
    }
}

int TBananaEditor::GetNextFreeBananaNumber()
{
    if (fBananaList->GetSize() == 0 && fBananaListTemp->GetSize() == 0)
        return 0;

    vector<int> banNumbers;
    TIter next_list(fBananaList);
    while (TObject *obj = next_list())
    {
        string str = obj->GetName();
        // get the number substring
        string r = str.substr(str.find_last_of("_") + 1);
        try
        {
            banNumbers.push_back(stoi(r));
        }
        catch (const std::exception &)
        {
        }
    }
    TIter next_temp(fBananaListTemp);
    while (TObject *obj = next_temp())
    {
        string str = obj->GetName();
        // get the number substring
        string r = str.substr(str.find_last_of("_") + 1);
        try
        {
            banNumbers.push_back(stoi(r));
        }
        catch (const std::exception &)
        {
        }
    }

    if (banNumbers.empty())
        return 0;

    int max = *max_element(banNumbers.begin(), banNumbers.end());
    return max + 1;
}

std::string TBananaEditor::GetDefaultBananaNamePrefix() const
{
    std::string prefix;
    if (fDefaultBananaNameEntry)
    {
        prefix = fDefaultBananaNameEntry->GetText();
    }

    if (prefix.empty())
    {
        prefix = fDefaultBananaNamePrefix;
    }

    if (prefix.empty())
    {
        prefix = fHist ? std::string(fHist->GetName()) + "_ban" : "banana";
    }

    return prefix;
}

void TBananaEditor::Init(const TGWindow *p, UInt_t w, UInt_t h, TH1 *histo, TList *graphBananaList, Int_t id)
{
    //   cout << "graphBananaList: " << graphBananaList->GetSize() << endl;
    if (!histo || !graphBananaList)
    {
        cout << "***TBananaEditor::Init()" << endl << "Fed by empty histogram or bananaList!!" << endl;
        return;
    }

    fID = id;
    AdjustContextMenu();
    fBananaEditStatus = kNothing;
    fHist = histo;
    if (fSaveFileName.empty()) // SaveToFile was not called, use passed-in list
    {
        fBananaList = graphBananaList;
        fBananaListTemp = (TList *)graphBananaList->Clone("fBananaListTemp");
    }

    fGraph = 0;

    /*testing*/
    if (!histo)
    {
        TH1D *h1 = new TH1D("h1", "h1", 1000, -2, 2);
        h1->FillRandom("gaus", 100000);
        // fHist = (TH1*)h1;

        TH2F *h2 = new TH2F("h2", "h2", 1000, 0., 20., 1000, 0., 20.);
        TF2 *xyg = new TF2("xyg", "xygaus", 0, 10, 0, 10);
        xyg->SetParameters(1, 5, 2, 5, 2); // amplitude, meanx,sigmax,meany,sigmay
        h2->FillRandom("xyg", 100000);
        fHist = (TH1 *)h2;
    }

    //   fMain = new TGMainFrame(0, w, h);
    fMain = new TGMainFrame((p != nullptr ? p : gClient->GetRoot()), w, h);
    fMain->SetCleanup(kDeepCleanup);
    fMain->Connect("CloseWindow()", "TBananaEditor", this, "DoExit()");
    fMain->DontCallClose(); // to avoid double deletions.
    // Create the embedded canvas
    fEcan = new TRootEmbeddedCanvas(0, fMain, w - 50, h - 50);
    Int_t wid = fEcan->GetCanvasWindowId();
    fCanvas = new TCanvas("Banana crafting canvas", 10, 10, wid);
    fEcan->AdoptCanvas(fCanvas);
    fCanvas->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TBananaEditor", this,
                     "EventInfo(Int_t,Int_t,Int_t,TObject*)");
    // key interactivity
    fCanvas->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TBananaEditor", this,
                     "KeyInteract(Int_t,Int_t,Int_t,TObject*)");

    // status bar
    Int_t parts[] = {45, 15, 10, 30};
    fStatusBar = new TGStatusBar(fMain, 50, 10, kVerticalFrame);
    fStatusBar->SetParts(parts, 4);
    fStatusBar->Draw3DCorner(kFALSE);

    // Create a horizontal frames
    // hiden frame with save and cancel buttons

    Pixel_t doneColor, cancelColor, exitColor, whiteColor;
    gClient->GetColorByName("#558B2F", doneColor);   // Olive green (matches avocado theme)
    gClient->GetColorByName("#8D6E63", cancelColor); // Warm brown (earth tone)
    gClient->GetColorByName("#B71C1C", exitColor);
    gClient->GetColorByName("#FFFFFF", whiteColor);

    hframe0 = new TGHorizontalFrame(fMain, 200, 40);

    fBtnOK = new TGTextButton(hframe0, "Do&ne");
    fBtnOK->ChangeBackground(doneColor);
    fBtnOK->Connect("Clicked()", "TBananaEditor", this, "OkAction()");
    hframe0->AddFrame(fBtnOK, new TGLayoutHints(kLHintsLeft, 4, 4, 4, 4));

    fBtnCancel = new TGTextButton(hframe0, "&Cancel ");
    fBtnCancel->ChangeBackground(cancelColor);
    fBtnCancel->Connect("Clicked()", "TBananaEditor", this, "CancelAction()");
    hframe0->AddFrame(fBtnCancel, new TGLayoutHints(kLHintsRight, 4, 4, 4, 4));

    // add banana frame
    hframe1 = new TGHorizontalFrame(fMain, 200, 40);

    TGHorizontalFrame *bananaControlFrame = new TGHorizontalFrame(hframe1, 200, 40);

    fBtnNewBanana = new TGTextButton(bananaControlFrame, "&Add new banana");
    fBtnNewBanana->Connect("Clicked()", "TBananaEditor", this, "AddNewBanana()");
    bananaControlFrame->AddFrame(fBtnNewBanana, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    fBtnEditBanana = new TGTextButton(bananaControlFrame, "&Edit banana");
    fBtnEditBanana->Connect("Clicked()", "TBananaEditor", this, "DoEditBanana()");
    bananaControlFrame->AddFrame(fBtnEditBanana, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    bananaControlFrame->AddFrame(new TGLabel(bananaControlFrame, "Default banana name"),
                                 new TGLayoutHints(kLHintsCenterY, 14, 4, 3, 4));
    if (fDefaultBananaNamePrefix.empty())
    {
        fDefaultBananaNamePrefix = Form("%s_ban", fHist->GetName());
    }
    fDefaultBananaNameEntry = new TGTextEntry(bananaControlFrame, fDefaultBananaNamePrefix.c_str());
    fDefaultBananaNameEntry->SetWidth(180);
    bananaControlFrame->AddFrame(fDefaultBananaNameEntry, new TGLayoutHints(kLHintsCenterY, 4, 5, 3, 4));

    hframe1->AddFrame(bananaControlFrame, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 0, 0, 0));

    // fBtnSave = new TGTextButton(hframe1, "&Save & Send");
    // fBtnSave->Connect("Clicked()", "TBananaEditor", this, "DoSave()");
    // hframe1->AddFrame(fBtnSave, new TGLayoutHints(kLHintsLeft, 5, 5, 3, 4));
    // fBtnSave->SetState(kButtonDisabled);

    hframe1->AddFrame(new TGHorizontalFrame(hframe1), new TGLayoutHints(kLHintsExpandX));

    fBtnExit = new TGTextButton(hframe1, "&Exit editor");
    fBtnExit->ChangeBackground(exitColor);
    fBtnExit->SetTextColor(whiteColor);
    fBtnExit->Connect("Clicked()", "TBananaEditor", this, "DoExit()");
    hframe1->AddFrame(fBtnExit, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 3, 4));

    //
    fMain->AddFrame(hframe0, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
    fMain->AddFrame(fEcan, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY));
    fMain->AddFrame(fStatusBar, new TGLayoutHints(kLHintsExpandX, 5, 5, 10, 0));
    fMain->AddFrame(hframe1, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
    // Set a name to the main frame
    fMain->SetWindowName("Banana editor");
    fMain->MapSubwindows();

    // Initialize the layout algorithm via Resize()
    fMain->Resize(fMain->GetDefaultSize());

    // Map main frame
    fMain->MapWindow();
    fMain->HideFrame(hframe0);

    DrawDefaultCanvas();

    gClient->WaitFor(fMain);
}

TBananaEditor::~TBananaEditor()
{
}
