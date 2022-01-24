// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include <wx/app.h>
#include <wx/object.h>
#include <wx/clipbrd.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/statusbr.h>
#include <wx/msgdlg.h>
#include <wx/icon.h>
#include <wx/dcclient.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/gbsizer.h>
#include <wx/splitter.h>
#include <wx/notebook.h>

#include <wx/docking/docking.h>

#include "../sample.xpm"

#if USE_XPM_BITMAPS
#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/copy.xpm"
#include "bitmaps/cut.xpm"
#include "bitmaps/past.xpm"
#include "bitmaps/help.xpm"
#endif // USE_XPM_BITMAPS

#if wxUSE_DOCKING

enum
{
	Tool_New,
	Tool_Open,
	Tool_Save,
	Tool_Copy,
	Tool_Cut,
	Tool_Paste,
	Tool_Help,
	Tool_Max
};

// -- application --
static const long ToolbarDefaultStyle = wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT | wxTB_NO_TOOLTIPS;
static wxBitmap toolBarBitmaps[Tool_Max];

class MyApp : public wxApp
{
public:
	bool OnInit() wxOVERRIDE;
};

wxDECLARE_APP(MyApp);
wxIMPLEMENT_APP(MyApp);

// -- wxSizeReportCtrl -- (an utility control that always reports it's client size)
class wxSizeReportCtrl : public wxControl
{
public:
	wxSizeReportCtrl(wxWindow *parent, wxString const &text, wxDockingFrame *frame, wxWindowID id = wxID_ANY,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize
		)
	: wxControl(parent, id, pos, size, wxNO_BORDER)
	, m_text(text)
	, m_frame(frame)
	{
	}

private:
	void OnPaint(wxPaintEvent &WXUNUSED(evt))
	{
		wxPaintDC dc(this);
		wxSize size = GetClientSize();
		wxString s;
		int h, w, height, x, y;

		dc.SetFont(*wxNORMAL_FONT);
		dc.SetBrush(*wxWHITE_BRUSH);
		dc.SetPen(*wxWHITE_PEN);
		dc.DrawRectangle(0, 0, size.x, size.y);
		dc.SetPen(*wxLIGHT_GREY_PEN);
		dc.DrawLine(0, 0, size.x, size.y);
		dc.DrawLine(0, size.y, size.x, 0);

		s = GetWindowDescription(dc, this, w, height)+m_text;
		dc.GetTextExtent(s, &w, &height);

		height += FromDIP(3);
		x = (size.x - w) / 2;
		y = ((size.y - (height * 5)) / 2);
		dc.DrawText(s, x, y);
		y += height+2;

		wxWindow *window = GetParent();
		if (window)
		{
			s = GetWindowDescription(dc, window, w, h);
			dc.DrawText(s, x, y);
			y += height + 2;

			window = window->GetParent();
			if (window)
			{
				s = GetWindowDescription(dc, window, w, h);
				dc.DrawText(s, x, y);
				y += height + 2;
			}
		}
	}

	wxString GetWindowDescription(wxPaintDC &dc, wxWindow *window, int &w, int &h)
	{
		wxPoint pos = window->GetPosition();
		wxSize sz = window->GetSize();
		wxString s;

		const char *type = "";
		wxString posStr;
		wxSplitterWindow *sp = dynamic_cast<wxSplitterWindow *>(window);
		if (sp != NULL)
		{
			const char *modeTxt;
			wxSplitMode mode = sp->GetSplitMode();
			wxWindow *w1 = sp->GetWindow1();
			wxWindow *w2 = sp->GetWindow2();
			const char *orientationTxt = "UNKN";

			if (mode == wxSPLIT_HORIZONTAL)
			{
				modeTxt = "HORZ";
				if (w1 == this)
					orientationTxt = "TOP";
				else if (w2 == this)
					orientationTxt = "BTM";
			}
			else
			{
				modeTxt = "VERT";
				if (w1 == this)
					orientationTxt = "LEFT";
				else if (w2 == this)
					orientationTxt = "RGHT";
			}

			type = "Splitter";
			posStr.Printf("Sash: %d", sp->GetSashPosition());
			posStr << " " << modeTxt << "/" << orientationTxt;
		}
		else if (dynamic_cast<wxNotebook *>(window) != NULL)
			type = "Notebook";
		else if (dynamic_cast<wxDockingFrame *>(window) != NULL)
			type = "Frame";

		s.Printf("%p Pos: %d/%d Size: %d x %d %s ", (void *)window, pos.x, pos.y, sz.x, sz.y, type);
		s += posStr;
		dc.GetTextExtent(s, &w, &h);

		return s;
	}

	void OnEraseBackground(wxEraseEvent &WXUNUSED(evt))
	{
		// intentionally empty
	}

	void OnSize(wxSizeEvent &WXUNUSED(evt))
	{
		Refresh();
	}

private:
	wxString m_text;
	wxDockingFrame *m_frame;

	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(wxSizeReportCtrl, wxControl)
	EVT_PAINT(wxSizeReportCtrl::OnPaint)
	EVT_SIZE(wxSizeReportCtrl::OnSize)
	EVT_ERASE_BACKGROUND(wxSizeReportCtrl::OnEraseBackground)
wxEND_EVENT_TABLE()

enum
{
	ID_Exit = wxID_EXIT,
	ID_About = wxID_ABOUT,

	ID_CreateTree = wxID_HIGHEST + 1,

	ID_LayoutSerialize,
	ID_LayoutDeserialize,
	ID_LayoutCopySerialize,
	ID_LayoutSplitCentral,
	ID_LayoutSplitLeft,
	ID_LayoutSplitRight,
	ID_LayoutSplitTop,
	ID_LayoutSplitBottom,
	ID_LayoutSplitLeftBorder,
	ID_LayoutSplitRightBorder,
	ID_LayoutSplitTopBorder,
	ID_LayoutSplitBottomBorder,
	ID_LayoutRemoveDockedPanel,

	ID_ToolbarLeftAdd,
	ID_ToolbarRightAdd,
	ID_ToolbarTopAdd,
	ID_ToolbarBottomAdd,

	ID_ToolbarLeftRemove,
	ID_ToolbarRightRemove,
	ID_ToolbarTopRemove,
	ID_ToolbarBottomRemove,

};

class MyFrame : public wxDockingFrame
{
public:
	MyFrame(wxWindow *parent,
			wxWindowID id,
			const wxString &title,
			const wxPoint &pos = wxDefaultPosition,
			const wxSize &size = wxDefaultSize,
			long style = wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);

public:
	void OnExit(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);

	void OnNewPanel(wxCommandEvent &evt);
	void OnNewPanelBorder(wxCommandEvent &evt);
	void OnLayoutSerialize(wxCommandEvent &evt);
	void OnLayoutDeserialize(wxCommandEvent &evt);
	void OnLayoutCopySerialize(wxCommandEvent &evt);
	void OnLayoutRemoveDockingPanel(wxCommandEvent &evt);

	void OnToolbar(wxCommandEvent &evt);
	wxToolBar *CreateDockingToolbar(bool top, bool left, wxDockingInfo &info);

	wxSizeReportCtrl *createSizeReportCtrl(wxString const &text);

	void createInitialLayout();
protected:
	wxMenu *createDockingMenu();
	wxMenu *createToolbarMenu();
	wxToolBar *horizontalToolBar(bool top);
	wxToolBar *verticalToolBar(bool left);
	void SetActivePanel(wxDockingPanel *panel) override;

private:
	uint32_t m_newPanel;

	wxDECLARE_EVENT_TABLE();
};

bool MyApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	MyFrame *frame = new MyFrame(NULL,
								 wxID_ANY,
								 "wxDocking Sample Application",
								 wxDefaultPosition,
								 wxWindow::FromDIP(wxSize(800, 600), NULL));
	frame->Show();
	//frame->Maximize();
	frame->createInitialLayout();

	return true;
}

wxBEGIN_EVENT_TABLE(MyFrame, wxDockingFrame)
	EVT_MENU(ID_Exit, MyFrame::OnExit)
	EVT_MENU(ID_About, MyFrame::OnAbout)

	EVT_MENU(ID_LayoutSerialize, MyFrame::OnLayoutSerialize)
	EVT_MENU(ID_LayoutDeserialize, MyFrame::OnLayoutDeserialize)
	EVT_MENU(ID_LayoutCopySerialize, MyFrame::OnLayoutCopySerialize)
	EVT_MENU(ID_LayoutSplitCentral, MyFrame::OnNewPanel)
	EVT_MENU(ID_LayoutSplitLeft, MyFrame::OnNewPanel)
	EVT_MENU(ID_LayoutSplitRight, MyFrame::OnNewPanel)
	EVT_MENU(ID_LayoutSplitTop, MyFrame::OnNewPanel)
	EVT_MENU(ID_LayoutSplitBottom, MyFrame::OnNewPanel)
	EVT_MENU(ID_LayoutSplitLeftBorder, MyFrame::OnNewPanelBorder)
	EVT_MENU(ID_LayoutSplitRightBorder, MyFrame::OnNewPanelBorder)
	EVT_MENU(ID_LayoutSplitTopBorder, MyFrame::OnNewPanelBorder)
	EVT_MENU(ID_LayoutSplitBottomBorder, MyFrame::OnNewPanelBorder)
	EVT_MENU(ID_LayoutRemoveDockedPanel, MyFrame::OnLayoutRemoveDockingPanel)


	EVT_MENU(ID_ToolbarLeftAdd, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarRightAdd, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarTopAdd, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarBottomAdd, MyFrame::OnToolbar)

	EVT_MENU(ID_ToolbarLeftRemove, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarRightRemove, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarTopRemove, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarBottomRemove, MyFrame::OnToolbar)
wxEND_EVENT_TABLE()

MyFrame::MyFrame(wxWindow *parent,
				 wxWindowID id,
				 const wxString &title,
				 const wxPoint &pos,
				 const wxSize &size,
				 long style)
: wxDockingFrame(parent, id, title, pos, size, style)
, m_newPanel(0)
{
	// set frame icon
	SetIcon(wxIcon(sample_xpm));

	// create menu
	wxMenuBar *mb = new wxMenuBar;

	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(wxID_EXIT);

	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(wxID_ABOUT);

	mb->Append(fileMenu, _("&File"));
	mb->Append(createDockingMenu(), _("&Docking"));
	mb->Append(createToolbarMenu(), _("&Toolbar"));
	mb->Append(helpMenu, _("&Help"));

	SetMenuBar(mb);

	CreateStatusBar();
	GetStatusBar()->SetStatusText(_("Ready"));

	SetMinSize(wxSize(200,150));

#if USE_XPM_BITMAPS
#define INIT_TOOL_BMP(bmp) \
        toolBarBitmaps[Tool_##bmp] = wxBitmap(bmp##_xpm)
#else // !USE_XPM_BITMAPS
#define INIT_TOOL_BMP(bmp) \
        toolBarBitmaps[Tool_##bmp] = wxBITMAP(bmp)
#endif // USE_XPM_BITMAPS/!USE_XPM_BITMAPS

	INIT_TOOL_BMP(New);
	INIT_TOOL_BMP(Open);
	INIT_TOOL_BMP(Save);
	INIT_TOOL_BMP(Copy);
	INIT_TOOL_BMP(Cut);
	INIT_TOOL_BMP(Paste);
	INIT_TOOL_BMP(Help);

	//createInitialLayout();
	Refresh();
}

wxMenu *MyFrame::createDockingMenu()
{
	wxMenu *menu = new wxMenu;

	wxMenu *submenu = new wxMenu;
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitCentral, wxString(wxT("Tab")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitLeft, wxString(wxT("Left")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitRight, wxString(wxT("Right")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitTop, wxString(wxT("Top")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitBottom, wxString(wxT("Bottom")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(menu, wxID_ANY, wxT("New panel"), wxEmptyString, wxITEM_NORMAL, submenu));
	submenu = new wxMenu;
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitLeftBorder, wxString(wxT("Left")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitRightBorder, wxString(wxT("Right")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitTopBorder, wxString(wxT("Top")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitBottomBorder, wxString(wxT("Bottom")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(menu, wxID_ANY, wxT("Split to border"), wxEmptyString, wxITEM_NORMAL, submenu));
	menu->Append(new wxMenuItem(menu, ID_LayoutRemoveDockedPanel, wxString(wxT("Remove panel")), wxEmptyString, wxITEM_NORMAL));
	menu->AppendSeparator();
	menu->Append(new wxMenuItem(submenu, ID_LayoutCopySerialize, wxString(wxT("Copy layout to Clipboard")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(submenu, ID_LayoutSerialize, wxString(wxT("Save layout")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(submenu, ID_LayoutDeserialize, wxString(wxT("Restore layout")), wxEmptyString, wxITEM_NORMAL));

	return menu;
}

wxMenu *MyFrame::createToolbarMenu()
{
	wxMenu *menu = new wxMenu;

	wxMenu *submenu = new wxMenu;
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarLeftAdd, wxString(wxT("Left")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarRightAdd, wxString(wxT("Right")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarTopAdd, wxString(wxT("Top")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarBottomAdd, wxString(wxT("Bottom")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(menu, wxID_ANY, wxT("Add"), wxEmptyString, wxITEM_NORMAL, submenu));

	submenu = new wxMenu;
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarLeftRemove, wxString(wxT("Left")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarRightRemove, wxString(wxT("Right")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarTopRemove, wxString(wxT("Top")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarBottomRemove, wxString(wxT("Bottom")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(menu, wxID_ANY, wxT("Remove"), wxEmptyString, wxITEM_NORMAL, submenu));

	return menu;
}

void MyFrame::SetActivePanel(wxDockingPanel *panel)
{
	wxDockingFrame::SetActivePanel(panel);
	panel = GetActivePanel();

	wxString s = "Active panel: ";
	if (panel)
	{
		s << panel->GetLabel();
		//s << " (" << panel->m_type << ")";
	}
	else
		s += "UNKNOWN";

	s << " - " << (void *)panel;
	//GetStatusBar()->SetStatusText(s);
}

wxSizeReportCtrl *MyFrame::createSizeReportCtrl(wxString const &text)
{
	return new wxSizeReportCtrl(this, text, this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
}

wxToolBar *MyFrame::verticalToolBar(bool left)
{
	long style = wxTB_VERTICAL| ToolbarDefaultStyle;

	// Even if the toolbar is at the bottom or right, we may not
	// specify this here. It must always be set to TOP/LEFT.
	if (left)
		style |= wxTB_LEFT;
	//else
	//	style |= wxTB_RIGHT;

	wxSize sz = GetClientSize();
	sz.SetWidth(20);

	wxToolBar *tb = new wxToolBar(this, wxID_ANY, wxPoint(0, 0), wxDefaultSize, style);
	tb->AddRadioTool(wxID_ANY, "First", wxBITMAP(New));
	tb->AddRadioTool(wxID_ANY, "Second", wxBITMAP(Open));
	tb->AddRadioTool(wxID_ANY, "Third", wxBITMAP(Save));
	tb->AddSeparator();
	tb->AddTool(wxID_HELP, "Help", wxBITMAP(Help));
	tb->AddTool(wxID_ANY, "Cut", wxBITMAP(Cut));
	tb->AddTool(wxID_ANY, "Paste", wxBITMAP(Paste));

	tb->Realize();

	return tb;
}

wxToolBar *MyFrame::horizontalToolBar(bool top)
{
	long style = wxTB_HORIZONTAL | ToolbarDefaultStyle;

	// Even if the toolbar is at the bottom or right, we may not
	// specify this here. It must always be set to TOP/LEFT.
	if (top)
		style |= wxTB_TOP;
	//else
	//	style |= wxTB_BOTTOM;

	wxToolBar *tb = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);

	tb->AddTool(wxID_ANY, "Copy", wxBITMAP(Copy));
	tb->AddTool(wxID_ANY, "Paste", wxBITMAP(Paste));
	tb->AddSeparator();

	wxStaticText *st;
	st = new wxStaticText(tb, wxID_ANY, wxT("Columns:"), wxDefaultPosition, wxDefaultSize, 0);
	st->Wrap(-1);
	tb->AddControl(st);

	wxComboBox *box = new wxComboBox(tb, wxID_ANY, wxT("Auto"));
	box->Append(wxT("Auto"));
	box->Append(wxT("8"));
	box->Append(wxT("16"));
	box->SetSelection(0);
	tb->AddControl(box);


	st = new wxStaticText(tb, wxID_ANY, wxT("Size"), wxDefaultPosition, wxDefaultSize, 0);
	st->Wrap(-1);
	tb->AddControl(st);
	wxTextCtrl *txt = new wxTextCtrl(tb, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxBORDER_STATIC);
	tb->AddControl(txt);


	wxCheckBox *chk = new wxCheckBox(tb, wxID_ANY, wxT("Spaces"), wxDefaultPosition, wxDefaultSize, 0);
	chk->SetValue(true);
	tb->AddControl(chk);
	tb->AddSeparator();

	wxButton *btn = new wxButton(tb, wxID_ANY, wxT("New"), wxDefaultPosition, wxDefaultSize, 0);
	tb->AddControl(btn);

	tb->Realize();

	return tb;
}

void MyFrame::OnExit(wxCommandEvent &WXUNUSED(event))
{
	Close(true);
}

void MyFrame::OnAbout(wxCommandEvent &WXUNUSED(event))
{
	wxString title = _("About wxDockingPanel Demo");
	wxString text = _("wxDockingPanel Demo\nAdemonstration for dockable windows in wxWidgets\n(c) Copyright by Gerhard Gruber");

	wxMessageBox(text, title, wxOK, this);
}

wxToolBar *MyFrame::CreateDockingToolbar(bool horizontal, bool topleft, wxDockingInfo &info)
{
	wxString title = "Toolbar ";
	wxToolBar *tb = NULL;
	if (horizontal)
	{
		tb = horizontalToolBar(topleft);
		if (topleft)
		{
			info.SetDirection(wxTOP);
			title += "top";
		}
		else
		{
			info.SetDirection(wxBOTTOM);
			title += "bottom";
		}
	}
	else
	{
		tb = verticalToolBar(topleft);
		if (topleft)
		{
			info.SetDirection(wxLEFT);
			title += "left";
		}
		else
		{
			info.SetDirection(wxRIGHT);
			title += "right";
		}
	}

	info.SetTitle(title);

	return tb;
}

void MyFrame::OnToolbar(wxCommandEvent &event)
{
	wxString title = "Toolbar ";
	wxDockingInfo info;
	bool add = true;

	switch (event.GetId())
	{
		case ID_ToolbarLeftRemove:
			add = false;
			wxFALLTHROUGH;
		case ID_ToolbarLeftAdd:
			info.SetDirection(wxLEFT);
			title += "left";
		break;

		case ID_ToolbarRightRemove:
			add = false;
			wxFALLTHROUGH;
		case ID_ToolbarRightAdd:
			info.SetDirection(wxRIGHT);
			title += "right";
		break;

		case ID_ToolbarTopRemove:
			add = false;
			wxFALLTHROUGH;
		case ID_ToolbarTopAdd:
			info.SetDirection(wxTOP);
			title += "top";
		break;

		case ID_ToolbarBottomRemove:
			add = false;
			wxFALLTHROUGH;
		case ID_ToolbarBottomAdd:
			info.SetDirection(wxBOTTOM);
			title += "bottom";
		break;
	}

	wxToolBar *tb = NULL;
	if (add)
	{
#pragma WARNING("Toolbar code missing")
/*		bool top = info.GetDirection() == wxTOP;
		bool left = info.GetDirection() == wxLEFT;

		tb = horizontalToolBar(top);
		tb = verticalToolBar(left);

		AddToolBar(tb, info);*/
	}
	else
		RemoveToolBar(NULL, info);
}

void MyFrame::OnLayoutCopySerialize(wxCommandEvent &)
{
#if wxUSE_CLIPBOARD
	//if (wxTheClipboard->Open())
	//{
	//	wxTheClipboard->SetData(new wxTextDataObject(s));
	//	wxTheClipboard->Close();
	//}
#endif
}

void MyFrame::OnLayoutSerialize(wxCommandEvent &)
{
}

void MyFrame::OnLayoutDeserialize(wxCommandEvent &)
{
}

void MyFrame::OnNewPanel(wxCommandEvent &event)
{
	wxDockingPanel *active = GetActivePanel();
	if (!active)
		return;

	wxString title = "New Panel: ";
	title << ++m_newPanel;
	wxDockingInfo info(title);
	info.SetPanel(active);

	switch (event.GetId())
	{
		case ID_LayoutSplitCentral:
			info.SetDirection(wxCENTRAL);
		break;

		case ID_LayoutSplitLeft:
			info.SetDirection(wxLEFT);
		break;

		case ID_LayoutSplitRight:
			info.SetDirection(wxRIGHT);
		break;

		case ID_LayoutSplitTop:
			info.SetDirection(wxTOP);
		break;

		case ID_LayoutSplitBottom:
			info.SetDirection(wxBOTTOM);
		break;
	}

	wxSizeReportCtrl *panel = createSizeReportCtrl(title);
	if (info.GetDirection() == wxCENTRAL)
		AddTabPanel(panel, info);
	else
		SplitPanel(panel, info);
}

void MyFrame::OnNewPanelBorder(wxCommandEvent &event)
{
	wxString title = "New Borderpanel: ";
	title << ++m_newPanel;
	wxDockingInfo info(title);
	info.SetPanel(NULL);		// nullpointer means to dock to the border

	switch (event.GetId())
	{
		case ID_LayoutSplitLeftBorder:
			info.SetDirection(wxLEFT);
		break;

		case ID_LayoutSplitRightBorder:
			info.SetDirection(wxRIGHT);
		break;

		case ID_LayoutSplitTopBorder:
			info.SetDirection(wxUP);
		break;

		case ID_LayoutSplitBottomBorder:
			info.SetDirection(wxDOWN);
		break;
	}

	SplitPanel(createSizeReportCtrl(title), info);
}

void MyFrame::OnLayoutRemoveDockingPanel(wxCommandEvent &evt)
{
	WXUNUSED(evt);

	wxDockingPanel *w = GetActivePanel();
	wxDockingPanel *a = RemovePanel(w);
	if (a)
		delete w;
}

void MyFrame::createInitialLayout()
{
	Defaults()
		.SetTabDirection(wxTOP)
	;

	wxSize leftSz(150,-1);
	wxSize rightSz(150,150);
	wxSize bottomSz(-1,100);

	wxDockingPanel *rootTab = AddTabPanel(createSizeReportCtrl("Ctrl1.1"), wxDockingInfo("Size Report 1.1"));
	AddTabPanel(createSizeReportCtrl("Ctrl1.2"), wxDockingInfo("Size Report 1.2").SetPanel(rootTab));
	AddTabPanel(createSizeReportCtrl("Ctrl1.2"), wxDockingInfo("Size Report 1.3").SetPanel(rootTab));

	wxDockingPanel *l = SplitPanel(createSizeReportCtrl("Ctrl2.0"), wxDockingInfo("Size Report 2.0").SetPanel(rootTab).SetDirection(wxLEFT).SetSize(leftSz));
	SplitPanel(createSizeReportCtrl("Ctrl3.0"), wxDockingInfo("Size Report 3.0").SetDirection(wxRIGHT).SetSize(rightSz));
	SplitPanel(createSizeReportCtrl("Ctrl4.0"), wxDockingInfo("Size Report 4.0").SetPanel(l).SetDirection(wxDOWN).SetSize(bottomSz));
	SplitPanel(createSizeReportCtrl("Ctrl5.0"), wxDockingInfo("Size Report 5.0").SetPanel(rootTab).SetDirection(wxDOWN).SetSize(bottomSz));
	SplitPanel(createSizeReportCtrl("Ctrl6.0"), wxDockingInfo("Size Report 6.0").SetDirection(wxLEFT).SetSize(leftSz));

	//wxSplitterWindow *sp = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
}

#else

#error "Demo cannot be compiled unless wxWidgets has been configured with wxUSE_DOCKING=1"

#endif
