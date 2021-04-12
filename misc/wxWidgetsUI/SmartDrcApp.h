#pragma once

#include "wx/app.h"
#include "wx/docview.h"

class cSmartDrcApp : public wxApp
{
   wxDocManager* m_docManager = nullptr;
   wxMenu* m_menuEdit = nullptr;
   wxVector<wxString> m_filesFromCmdLine;

   void OnInitCmdLine(wxCmdLineParser& parser);

   bool OnCmdLineParsed(wxCmdLineParser& parser) override;

   void CreateMenuBarForFrame(wxFrame* frame, wxMenu* file, wxMenu* edit);
   void AppendDocumentFileCommands(wxMenu* menu, bool supportsPrinting);

   void OnAbout(wxCommandEvent& WXUNUSED(event));

public:
   bool OnInit() override;
   wxFrame *CreateChildFrame(wxView *view);
};

wxDECLARE_APP(cSmartDrcApp);
