// wxWidgetsUI.cpp : Defines the entry point for the application.
//

#include "pch.h"

#include "wx/cmdline.h"
#include "wx/stockitem.h"
#include "wx/filesys.h"
#include "wx/config.h"

#include "SmartDrcFrame.h"
#include "SmartDrcDoc.h"
#include "SmartDrcApp.h"

void cSmartDrcApp::OnInitCmdLine(wxCmdLineParser &parser)
{
   wxApp::OnInitCmdLine(parser);
   parser.AddParam("document-file", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL);
}

bool cSmartDrcApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
   // save any files given on the command line: we'll open them in OnInit()
   // later, after creating the frame
   for (size_t i = 0; i != parser.GetParamCount(); ++i) {
      m_filesFromCmdLine.push_back(parser.GetParam(i));
   }

   return wxApp::OnCmdLineParsed(parser);
}

void cSmartDrcApp::CreateMenuBarForFrame(wxFrame *frame, wxMenu *file, wxMenu *edit)
{
   wxMenuBar *menubar = new wxMenuBar;

   menubar->Append(file, wxGetStockLabel(wxID_FILE));

   if (edit) {
      menubar->Append(edit, wxGetStockLabel(wxID_EDIT));
   }

   wxMenu *help = new wxMenu;
   help->Append(wxID_ABOUT);
   menubar->Append(help, wxGetStockLabel(wxID_HELP));

   frame->SetMenuBar(menubar);
}

void cSmartDrcApp::AppendDocumentFileCommands(wxMenu *menu, bool supportsPrinting)
{
   menu->Append(wxID_CLOSE);
   menu->Append(wxID_SAVE);
   menu->Append(wxID_SAVEAS);
   menu->Append(wxID_REVERT, _("Re&vert..."));

   if (supportsPrinting) {
      menu->AppendSeparator();
      menu->Append(wxID_PRINT);
      menu->Append(wxID_PRINT_SETUP, "Print &Setup...");
      menu->Append(wxID_PREVIEW);
   }
}

wxFrame* cSmartDrcApp::CreateChildFrame(wxView *view)
{
   using cFrameAny = wxDocChildFrameAny<wxAuiMDIChildFrame, wxAuiMDIParentFrame>;

   // create a child frame of appropriate class for the current mode
   wxDocument *doc = view->GetDocument();
   wxFileName path = doc->GetFilename();
   auto frame = wxStaticCast(GetTopWindow(), wxAuiMDIParentFrame);
   auto subframe = new cFrameAny(doc, view, frame, wxID_ANY, path.GetName(), wxDefaultPosition, wxSize(300, 300));

   auto menuFile = new wxMenu;
   menuFile->Append(wxID_NEW);
   menuFile->Append(wxID_OPEN);
   AppendDocumentFileCommands(menuFile, true);
   menuFile->AppendSeparator();
   menuFile->Append(wxID_EXIT);

   auto menuEdit = new wxMenu;
   menuEdit->Append(wxID_COPY);
   menuEdit->Append(wxID_PASTE);
   menuEdit->Append(wxID_SELECTALL);

   CreateMenuBarForFrame(subframe, menuFile, menuEdit);

   subframe->SetIcon(wxICON(chrt));

   return subframe;
}

void cSmartDrcApp::OnAbout(wxCommandEvent &WXUNUSED(event))
{
   //...
}

bool cSmartDrcApp::OnInit()
{
   if (!wxApp::OnInit()) {
      return false;
   }

   SetAppName("SmartDRC Application");
   SetAppDisplayName("SmartDRC");

   m_docManager = new wxDocManager;

   new wxDocTemplate(m_docManager, "PCB Design", "*.xmlpcb", "", "xmlpcb", "PCB Design Doc", "PCB Design View",
      CLASSINFO(cPcbDesignDocument), CLASSINFO(cSmartDrcBoardView));

   auto frame = new cSmartDrcFrame(m_docManager, wxID_ANY, GetAppDisplayName(), wxDefaultPosition, wxSize(500, 400));

   auto menuFile = new wxMenu;

   menuFile->Append(wxID_NEW);
   menuFile->Append(wxID_OPEN);

   menuFile->AppendSeparator();
   menuFile->Append(wxID_EXIT);

   // A nice touch: a history of files visited. Use this menu.
   m_docManager->FileHistoryUseMenu(menuFile);
#if wxUSE_CONFIG
   m_docManager->FileHistoryLoad(*wxConfig::Get());
#endif // wxUSE_CONFIG

   CreateMenuBarForFrame(frame, menuFile, m_menuEdit);

   frame->SetIcon(wxICON(doc));
   frame->Centre();
   frame->Show();

   for (size_t i = 0; i != m_filesFromCmdLine.size(); ++i) {
      m_docManager->CreateDocument(m_filesFromCmdLine[i], wxDOC_SILENT);
   }

   return true;
}

wxIMPLEMENT_APP(cSmartDrcApp);
