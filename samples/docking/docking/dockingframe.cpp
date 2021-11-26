
#if wxUSE_DOCKING

#include <wx/gbsizer.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/toolbar.h>

#include <wx/app.h>
#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingframe.h>

using namespace std;

wxDockingInfo *wxDockingInfo::m_default = nullptr;

namespace
{
	// If a panel is floating, we have to keep track of it
	// so we can properly de-/serialize the layout.
	// this list will contain only floating frames.
	vector<wxDockingFrame *> gFrames;
};

wxIMPLEMENT_DYNAMIC_CLASS(wxDockingFrame, wxFrame);

wxDockingFrame::wxDockingFrame()
{
	init();
}

wxDockingFrame::wxDockingFrame(wxWindow *parent,
	wxWindowID id,
	const wxString &title,
	const wxPoint &pos,
	const wxSize &size,
	long style,
	const wxString &name)
: wxFrame()
{
	init();
	Create(parent, id, title, pos, size, style, name);
}
wxDockingFrame::~wxDockingFrame()
{
	UnbindEventHandlers();

	// Remove this frame from the list.
	for (vector<wxDockingFrame *>::iterator it = gFrames.begin(); it != gFrames.end(); ++it)
	{
		if (*it == this)
		{
			gFrames.erase(it);
			break;
		}
	}
}

void wxDockingFrame::init(void)
{
	m_rootPanel = nullptr;
	m_sizer = nullptr;

	m_toolbarsLeft = nullptr;
	m_toolbarsRight = nullptr;
	m_toolbarsTop = nullptr;
	m_toolbarsBottom = nullptr;

	m_defaults.defaults();
}

bool wxDockingFrame::Create(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
{
	m_activePanel = nullptr;

	if (!wxFrame::Create(parent, id, title, pos, size, style, name))
		return false;

	// Register the frame in our list. Should be done first thing after initialzing our frame.
	gFrames.push_back(this);

	m_sizer = new wxGridBagSizer(0, 0);
	m_sizer->SetFlexibleDirection(wxBOTH);
	m_sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	SetSizer(m_sizer);
	UpdateToolbarLayout();

	BindEventHandlers();

	return true;
}

void wxDockingFrame::BindEventHandlers(void)
{
	// We only bind our event handlers for the main frame.
	if (gFrames[0] != this)
		return;

	wxApp *app = static_cast<wxApp *>(wxApp::GetInstance());

	app->Bind(wxEVT_LEFT_DOWN, &wxDockingFrame::OnMouseLeftDown, this);
	app->Bind(wxEVT_SIZE, &wxDockingFrame::OnSize, this);
	app->Bind(wxEVT_SIZING, &wxDockingFrame::OnSize, this);
}

void wxDockingFrame::UnbindEventHandlers(void)
{
	if (gFrames[0] != this)
		return;

	wxApp *app = static_cast<wxApp *>(wxApp::GetInstance());

	app->Unbind(wxEVT_LEFT_DOWN, &wxDockingFrame::OnMouseLeftDown, this);
	app->Unbind(wxEVT_SIZE, &wxDockingFrame::OnSize, this);
	app->Unbind(wxEVT_SIZING, &wxDockingFrame::OnSize, this);
}

void wxDockingFrame::OnSize(wxSizeEvent &event)
{
	DoSize();
	event.Skip();
}

void wxDockingFrame::DoSize(void)
{
	if (m_rootPanel)
	{
		wxSize sz = GetClientSize();
		m_rootPanel->SetSize(sz);
	}
}

void wxDockingFrame::OnMouseLeftDown(wxMouseEvent &event)
{
	wxPoint mousePos = ::wxGetMousePosition();

	wxWindow *w = wxFindWindowAtPoint(mousePos);
	wxDockingPanel *p = FindDockingPanel(w);
	if (p)
		SetActivePanel(p);

	event.Skip();
}

void wxDockingFrame::SetActivePanel(wxDockingPanel *panel)
{
	m_activePanel = panel;
}

bool wxDockingFrame::isDockable(wxWindow *window) const
{
	for (int i = 0; i < 2; i++)
	{
		if (!window)
			return false;

		wxNotebook *p = dynamic_cast<wxNotebook *>(window);
		if (p)
			return true;

		wxSplitterWindow *s = dynamic_cast<wxSplitterWindow *>(window);
		if (s)
			return true;

		window = window->GetParent();
	}

	return false;
}

wxDockingPanel *wxDockingFrame::FindDockingPanel(wxWindow *window) const
{
	while (window)
	{
		if (isDockable(window))
			return window;

		window = window->GetParent();
	}

	return nullptr;
}

wxDockingPanel *wxDockingFrame::FloatPanel(wxWindow *panel, wxDockingInfo const &info)
{
	long style = wxSYSTEM_MENU | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX | wxCAPTION | wxCLIP_CHILDREN;

	wxDockingPanel *dp = nullptr;
/*	wxDockingFrame *frame = new wxDockingFrame(this, wxID_ANY, info.title(), info.position(), info.size(), style);
	wxDockingPanel *dp = frame->GetRootPanel();
	dp->SetTitle(info.title());
	panel->Reparent(dp);
	dp->SetFloatingWindow(frame);
	frame->Show();*/

	return dp;
}

/*
wxString wxDockingFrame::SerializeLayout(void) const
{
	// If all frames should be serialized, we start at the root frame.
	if (gFrames[0] != this)
		return gFrames[0]->SerializeLayout();

	wxString layout = SerializeFrame();
	if (layout.empty())
		return layout;

	// TODO: Serialize this frame here.

	return layout;
}

wxString wxDockingFrame::SerializeFrame(void) const
{
	wxString layout;

	return layout;
}

bool wxDockingFrame::DeserializeLayout(wxString layout)
{
	// If all frames should be deserialized, we start at the root frame.
	if (gFrames[0] != this)
		return gFrames[0]->DeserializeLayout(layout);

	return DeserializeFrame(layout);
}

bool wxDockingFrame::DeserializeFrame(wxString layout)
{
	return true;
}
*/

void wxDockingFrame::UpdateToolbarLayout(void)
{
	SetSizer(nullptr, true);

/*	m_sizer = new wxGridBagSizer(0, 0);
	m_sizer->SetFlexibleDirection(wxBOTH);
	m_sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	int top = m_toolbarsTop != nullptr;
	int left = m_toolbarsLeft != nullptr;
	int right = 0;
	int border = 0;

	if (m_toolbarsLeft)
		m_sizer->Add(m_toolbarsLeft, wxGBPosition(top, 0), wxGBSpan(1, 1), wxEXPAND, border);

	if (m_toolbarsRight)
	{
		m_sizer->Add(m_toolbarsRight, wxGBPosition(top, 1+left), wxGBSpan(1, 1), wxEXPAND, border);
		right = 1;
	}

	if (m_toolbarsTop)
		m_sizer->Add(m_toolbarsTop, wxGBPosition(0, 0), wxGBSpan(1, 1+left+right), wxEXPAND, border);

	if (m_toolbarsBottom)
		m_sizer->Add(m_toolbarsBottom, wxGBPosition(1+top, 0), wxGBSpan(1, 1+left+right), wxEXPAND, border);

	if (m_rootPanel)
	{
		wxGBPosition rootPos(top, left);
		m_sizer->Add(m_rootPanel, rootPos, wxGBSpan(1, 1), wxEXPAND, border);

		m_sizer->AddGrowableCol(rootPos.GetCol());
		m_sizer->AddGrowableRow(rootPos.GetRow());

		SetSizer(m_sizer, true);
		Layout();
		m_rootPanel->Update();
	}*/
}

wxDockingPanel *wxDockingFrame::AddToolBar(wxToolBar *toolbar, wxDockingInfo const &info)
{
	if (!toolbar)
		return nullptr;

	wxDockingPanel *dp = nullptr;

/*	wxSize tbSize = toolbar->GetSize();

	if (info.isToolbarHorizontal())
	{
		if (info.direction() == wxTOP)
		{
			if (!m_toolbarsTop)
				m_toolbarsTop = new wxDockingPanel(this, wxID_ANY);

			dp = m_toolbarsTop;
		}
		else
		{
			if (!m_toolbarsBottom)
				m_toolbarsBottom = new wxDockingPanel(this, wxID_ANY);

			dp = m_toolbarsBottom;
		}

		tbSize.SetWidth(-1);
	}
	else
	{
		if (info.direction() == wxLEFT)
		{
			if (!m_toolbarsLeft)
				m_toolbarsLeft = new wxDockingPanel(this, wxID_ANY);

			dp = m_toolbarsLeft;
		}
		else
		{
			if (!m_toolbarsRight)
				m_toolbarsRight = new wxDockingPanel(this, wxID_ANY);

			dp = m_toolbarsRight;
		}

		tbSize.SetHeight(-1);
	}

	toolbar->Reparent(dp);

	// We need to call Realize again after reparenting, otherwise the
	// size of the toolbar will be wrong in some cases.
	dp->SetMinSize(tbSize);
	dp->SetMaxSize(tbSize);
	toolbar->Realize();

	UpdateToolbarLayout();*/

	return dp;
}

bool wxDockingFrame::HideToolbar(wxDockingPanel *&toolbar)
{
	if (toolbar)
	{
		RemoveChild(toolbar);
		delete toolbar;
		toolbar = nullptr;
		return true;
	}

	return false;
}

bool wxDockingFrame::RemoveToolBar(wxToolBar *toolbar, wxDockingInfo const &info)
{
	bool rc = false;
/*	if (info.isToolbarHorizontal())
	{
		if (info.direction() == wxTOP)
			rc = HideToolbar(m_toolbarsTop);
		else
			rc = HideToolbar(m_toolbarsBottom);
	}
	else
	{
		if (info.direction() == wxLEFT)
			rc = HideToolbar(m_toolbarsLeft);
		else
			rc = HideToolbar(m_toolbarsRight);
	}

	if (rc)
		UpdateToolbarLayout();*/

	return rc;
}

wxNotebook *wxDockingFrame::CreateNotebook(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
{
	return new wxNotebook(parent, id, pos, size, style);

}

void wxDockingFrame::DeleteNotebook(wxNotebook *notebook)
{
	delete notebook;
}

wxSplitterWindow *wxDockingFrame::CreateSplitter(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
{
	return new wxSplitterWindow(parent, id, pos, size, style);
}

void wxDockingFrame::DeleteSplitter(wxSplitterWindow *splitter)
{
	delete splitter;
}

wxDockingPanel *wxDockingFrame::AddTabPanel(wxWindow *userWindow, wxDockingInfo const &info)
{
	wxDockingPanel *dp = CreateTabPanel(userWindow, info, nullptr);

	if(dp)
		SetActivePanel(dp);

	return dp;
}

wxDockingPanel *wxDockingFrame::CreateTabPanel(wxWindow *userWindow, wxDockingInfo const &info, wxWindow *parent)
{
	wxCHECK_MSG(userWindow, nullptr, wxT("userWindow is a nullptr"));

	wxDockingPanel *dockingTarget = info.dock();

	if (!dockingTarget)
		dockingTarget = userWindow->GetParent();
	else
	{
		wxCHECK_MSG(isDockable(dockingTarget), nullptr, wxT("Docking target is not dockable for tab"));
	}

	if (!parent)
		parent = dockingTarget->GetParent();

	if (!parent)
		parent = dockingTarget;

	// If the docking target itself is a notebook we just add the new window as a page to it.
	wxNotebook *nb = dynamic_cast<wxNotebook *>(dockingTarget);
	if (!nb)
	{
		// If the dockingtarget is a page of a notebook, we have to remove that page
		// and replace it with a new notebook where that page is added to.
		nb = CreateNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, info.tabStyle());
		wxString title;

		wxNotebook *pnb = dynamic_cast<wxNotebook *>(parent);
		if (pnb)
		{
			int pageIndex = -1;

			pageIndex = pnb->FindPage(dockingTarget);
			if (pageIndex == wxNOT_FOUND)
				return nullptr;

			title = pnb->GetPageText(pageIndex);
			pnb->RemovePage(pageIndex);
			pnb->InsertPage(pageIndex, nb, title, true);
			dockingTarget->Reparent(nb);
			nb->AddPage(dockingTarget, title, true);
		}
		else
		{
			wxSplitterWindow *parentSplitter = dynamic_cast<wxSplitterWindow *>(parent);
			if (parentSplitter)
			{
				parentSplitter->ReplaceWindow(dockingTarget, nb);
				//title = info.title();
				dockingTarget->Reparent(nb);
				nb->AddPage(dockingTarget, title, true);
			}
		}

		// If the parent was not a notebook, we have to create a new one.
		if(!nb)
			nb = CreateNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, info.tabStyle());
	}

	userWindow->Reparent(nb);
	nb->AddPage(userWindow, info.title(), true);

	if (!m_rootPanel)
		m_rootPanel = nb;

	return nb;
}

wxNotebook *wxDockingFrame::ReplaceNotebookPage(wxNotebook *notebook, wxWindow *oldPage, int &index, wxDockingInfo const &info)
{
	if (!notebook)
	{
		index = wxNOT_FOUND;
		return nullptr;
	}

	index = notebook->FindPage(oldPage);
	if (index == wxNOT_FOUND)
		return nullptr;

	wxString title = notebook->GetPageText(index);
	wxNotebook *nb = CreateNotebook(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, info.tabStyle());
	notebook->RemovePage(index);
	oldPage->Reparent(nb);
	notebook->InsertPage(index, nb, title, true);
	nb->AddPage(oldPage, title, true);

	return nb;
}

wxDockingPanel *wxDockingFrame::SplitPanel(wxWindow *userWindow, wxDockingInfo const &info)
{
	wxDockingPanel *dockingTarget = info.dock();
	bool forceResize = false;

	if (!dockingTarget)
	{
		dockingTarget = m_rootPanel;
		forceResize = true;
	}

	wxCHECK_MSG(isDockable(dockingTarget), nullptr, wxT("Docking target is not dockable for splitter"));

	wxWindow *parent = dockingTarget->GetParent();
	wxSplitterWindow *splitter = CreateSplitter(parent, wxID_ANY);
	splitter->SetSashGravity(1.0);
	dockingTarget->Reparent(splitter);
	if (dockingTarget == m_rootPanel)
		m_rootPanel = splitter;

	wxDockingPanel *dp1 = dockingTarget;
	wxDockingPanel *dp2 = userWindow;

	// When the parent is a docking window, we have to remove the panel from it and replace it
	// with our new splitter window. The taken panel becomes part of the splitter then.
	wxSplitterWindow *parentSplitter = dynamic_cast<wxSplitterWindow *>(parent);
	if (parentSplitter)
		parentSplitter->ReplaceWindow(dockingTarget, splitter);
	else
	{
		wxNotebook *pnb = dynamic_cast<wxNotebook *>(parent);
		if (pnb)
		{
			int pageIndex = -1;

			pageIndex = pnb->FindPage(dockingTarget);
			if (pageIndex == wxNOT_FOUND)
				return nullptr;

			wxString title = pnb->GetPageText(pageIndex);
			wxNotebook *nb = CreateNotebook(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, info.tabStyle());
			pnb->RemovePage(pageIndex);
			dockingTarget->Reparent(nb);
			pnb->InsertPage(pageIndex, splitter, title, true);
			nb->AddPage(dockingTarget, title, true);
			dp1 = nb;
		}
	}

	wxDockingInfo childInfo = info;
	childInfo.dock(splitter);
	childInfo.activate();
	dp2->Reparent(splitter);
	wxSize sz = info.size();

	wxDirection direction = info.direction();
	if (direction == wxLEFT || direction == wxUP)
		swap(dp1, dp2);

	// Preserve sash...
	int preservedSash = -1;
	int sashPos = -1;
	wxSplitterWindow *dockingSplitter = dynamic_cast<wxSplitterWindow *>(dockingTarget);
	if (dockingSplitter)
		preservedSash = dockingSplitter->GetSashPosition();

	static const int defaultWidth = 30;
	int dec = 0;

	// If the docking target is a splitter, we have to preserve the sash position, so
	// after the split we can reset it to about the same as before. Otherwise it
	// may get screwed up during the split operation. The position will be not exact
	// because after the split the window sizes will be slightly different.
	if (direction == wxLEFT || direction == wxRIGHT)
	{
		// Get the width the user desired
		sashPos = sz.x;
		sz = dockingTarget->GetSize();
		if (sashPos == -1)
			sashPos = defaultWidth;

		if (direction == wxRIGHT)
			sashPos = sz.x - sashPos;

		splitter->SplitVertically(dp1, dp2, sashPos);
	}
	else
	{
		sashPos = sz.y;
		sz = dockingTarget->GetSize();
		if (sashPos == -1)
				sashPos = defaultWidth;

		if (direction == wxDOWN)
			sashPos = sz.y - sashPos;

		splitter->SplitHorizontally(dp1, dp2, sashPos);
	}

	splitter->SetSashPosition(sashPos);
	// ... and restore it.
	if (dockingSplitter)
		dockingSplitter->SetSashPosition(preservedSash);

	SetActivePanel(splitter);

	return splitter;
}

wxDockingPanel *wxDockingFrame::RemovePanel(wxWindow *userWindow)
{
	if (!userWindow)
		return nullptr;

	wxWindow *parent = userWindow->GetParent();
	if (parent == nullptr)
		return nullptr;

	wxWindow *newDockingTarget = parent->GetParent();

	if (!newDockingTarget)
		newDockingTarget = this;

	userWindow->Reparent(newDockingTarget);

	wxNotebook *nb = dynamic_cast<wxNotebook *>(parent);
	if (nb)
	{
		int pageIndex = -1;

		pageIndex = nb->FindPage(userWindow);
		if (pageIndex == wxNOT_FOUND)
			return nullptr;

		// If the last page is removed, the notebook still stays, which will act as a placeholder.
		// If the notebook should be removed, it must be removed specifically.
		nb->RemovePage(pageIndex);
		SetActivePanel(nb);

		return nb;
	}

	wxSplitterWindow *splitter = dynamic_cast<wxSplitterWindow *>(parent);
	if (splitter)
	{
		wxWindow *otherWindow = splitter->GetWindow1();
		if (otherWindow == userWindow)
			otherWindow = splitter->GetWindow2();

		splitter->Unsplit(userWindow);
		splitter->Unsplit(otherWindow);
		otherWindow->Reparent(newDockingTarget);

		wxSplitterWindow *parentSplitter = dynamic_cast<wxSplitterWindow *>(splitter->GetParent());
		if (parentSplitter)
			parentSplitter->ReplaceWindow(splitter, otherWindow);

		DeleteSplitter(splitter);

		wxDockingFrame *frame = dynamic_cast<wxDockingFrame *>(newDockingTarget);
		if (frame)
		{
			if (frame->m_rootPanel == splitter)
				frame->m_rootPanel = otherWindow;
		}

		SetActivePanel(newDockingTarget);

		return newDockingTarget;
	}

	return nullptr;
}

#endif // wxUSE_DOCKING
