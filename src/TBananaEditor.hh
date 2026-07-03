#ifndef TBANANA_EDITOR
#define TBANANA_EDITOR

#include <iostream>
#include <string>

#include "RQ_OBJECT.h"
#include <TApplication.h>
#include <TAxis.h>
#include <TCanvas.h>
#include <TClass.h>
#include <TClassMenuItem.h>
#include <TColor.h>
#include <TF1.h>
#include <TF2.h>
#include <TFile.h>
#include <TFrame.h>
#include <TGButton.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGInputDialog.h>
#include <TGLabel.h>
#include <TGMsgBox.h>
#include <TGStatusBar.h>
#include <TGTextEntry.h>
#include <TGraph.h>
#include <TH1.h>
#include <TH2.h>
#include <TKey.h>
#include <TPad.h>
#include <TROOT.h>
#include <TRandom.h>
#include <TRootEmbeddedCanvas.h>
#include <TStyle.h>
#include <TSystem.h>

using namespace std;

enum BananaEditStatus
{
    kNothing,
    kDeleteBanana,
    kAddBanana2D,
    kAddBanana1D,
    kEditBanana2D,
    kEditBanana1D
};

class TBananaEditor : public TObject
{
    RQ_OBJECT("TBananaEditor")

  private:
    TRootEmbeddedCanvas *fEcan; // AddFrame
    TCanvas *fCanvas;
    TGMainFrame *fMain;

    const char *fCollectionName; //!

    TGHorizontalFrame *hframe0; // AddFrame
    TGHorizontalFrame *hframe1; // AddFrame

    TGTextButton *fBtnNewBanana;    // AddFrame
    TGTextButton *fBtnEditBanana;   // AddFrame
    TGTextButton *fBtnDeleteBanana; // AddFrame

    TGTextButton *fBtnCancel; // AddFrame
    TGTextButton *fBtnOK;     // AddFrame
    TGTextButton *fBtnExit;   // AddFrame
    TGTextButton *fBtnSave;   // AddFrame
    TGTextEntry *fDefaultBananaNameEntry;

    TGStatusBar *fStatusBar; // AddFrame

    TH1 *fHist;
    TGraph *fGraph;

    TList *fBananaList;
    TList *fBananaListTemp;

    std::string fSaveFileName; // Store filename for saving on exit
    std::string fDefaultBananaNamePrefix;

    int selectedPoint;
    int fID;
    BananaEditStatus fBananaEditStatus;

  private:
    int GetNextFreeBananaNumber();
    std::string GetDefaultBananaNamePrefix() const;
    bool IsEdited();

  public:
    TBananaEditor(const char *colName);
    void Init(const TGWindow *p, UInt_t w, UInt_t h, TH1 *histo, TList *graphBananaList, Int_t id);
    virtual ~TBananaEditor();
    void DrawDefaultCanvas();
    void SetStatusText(const char *txt, Int_t pi);
    void EventInfo(Int_t event, Int_t px, Int_t py, TObject *selected);
    void ClosingBananaWindow(TBananaEditor *); // *SIGNAL*
    void AddNewBanana();
    void DrawBanana(TGraph *gr);
    void DrawAllBananas();
    void Make2DBanana(Int_t event, Int_t px, Int_t py, TObject *selected);
    void EditBanana(Int_t event, Int_t px, Int_t py, TObject *selected);
    void KeyInteract(Int_t event, Int_t px, Int_t py, TObject *selected);
    void ListedForHighlightPoint(TVirtualPad *, TObject *, Int_t, Int_t);

    void OkAction();
    void CancelAction();

    void DoEditBanana();
    void DoSave();
    void DoExit();
    void CloseWindow();

    void TryToClose();
    void SaveToFile(const std::string filename);
    void SetDefaultBananaName(const char *name);
    void SetDefaultBananaName(const std::string &name);

    void AdjustContextMenu();
    void ContextInsertPoint(TObject *obj);
    void ContextRemovePoint(TObject *obj);
    void ContextRenameBanana(TObject *obj);
    void ContextDeleteBanana(TObject *obj);
    void ContextCloneBanana(TObject *obj);
    void ContextUnZoomBoth(TObject *obj);

    // void NewBananasFromEditor(Int_t id,TList *bananaList, Bool_t isEdited); // *SIGNAL*
    void NewBananasFromEditor(TList *bananaList); //*SIGNAL*
    void ClosingBananaEditor(Int_t);              // *SIGNAL*

    static void MoveGraphPoints(TGraph *g, Double_t dx, Double_t dy);

    ClassDef(TBananaEditor, 3)
};

#endif
