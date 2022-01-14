
#if wxUSE_DOCKING

#include <wx/app.h>
#include <wx/gbsizer.h>
#include <wx/dcscreen.h>

#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingframe.h>
#include <wx/docking/dockingselector.h>

#ifndef wxUSE_NOTEBOOK
#error wxDockingFrame requires wxNotebook enabled
#endif

#ifndef wxUSE_SPLITTER
#error wxDockingFrame requires wxSplitterWindow enabled
#endif

using namespace std;

wxDockingInfo *wxDockingInfo::m_default = nullptr;

namespace
{
	int gSashPos = 0;


	// If a panel is floating, we have to keep track of it
	// so we can properly de-/serialize the layout.
	// this list will contain only floating frames.
	vector<wxDockingFrame *> gFrames;

	/**
	 * The original wxFindWindowAtPoint doesn't allow us to ignore a window, so here we are using our own
	 * in order to ignore the overlay window, we use to indicate the docking target.
	 */
	wxWindow *FindWindowAtPoint(wxWindow *w, wxPoint const &pos)
	{
		const wxWindowList& childs = w->GetChildren();
		for (wxWindowList::const_iterator it = childs.begin(); it != childs.end(); ++it)
		{
			w = *it;

			if (w->GetScreenRect().Contains(pos))
			{
				wxWindow *result = w;

				// Recursively find the innermost child.
				w = ::FindWindowAtPoint(w, pos);
				if (w != nullptr)
					result = w;

				return result;
			}
		}

		return nullptr;
	}
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
	m_selector = nullptr;

	m_mouseCaptured = false;

	m_dockingThreshold = 10;
	m_dockingWidth = 20;

	m_event.clear();
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

	app->Bind(wxEVT_LEFT_UP, &wxDockingFrame::OnMouseLeftUp, this);
	app->Bind(wxEVT_MOTION, &wxDockingFrame::OnMouseMove, this);

	app->Bind(wxEVT_SPLITTER_DOUBLECLICKED, &wxDockingFrame::OnSplitterDClick, this);
	app->Bind(wxEVT_SPLITTER_SASH_POS_CHANGED, &wxDockingFrame::OnSashPosChanged, this);
	app->Bind(wxEVT_SPLITTER_SASH_POS_CHANGING, &wxDockingFrame::OnSashPosChanging, this);

	app->Bind(wxEVT_SIZE, &wxDockingFrame::OnSize, this);
	app->Bind(wxEVT_SIZING, &wxDockingFrame::OnSize, this);
}

void wxDockingFrame::UnbindEventHandlers(void)
{
	if (gFrames[0] != this)
		return;

	wxApp *app = static_cast<wxApp *>(wxApp::GetInstance());

	app->Unbind(wxEVT_LEFT_UP, &wxDockingFrame::OnMouseLeftUp, this);
	app->Unbind(wxEVT_MOTION, &wxDockingFrame::OnMouseMove, this);

	app->Unbind(wxEVT_SPLITTER_DOUBLECLICKED, &wxDockingFrame::OnSplitterDClick, this);
	app->Unbind(wxEVT_SPLITTER_SASH_POS_CHANGED, &wxDockingFrame::OnSashPosChanged, this);
	app->Unbind(wxEVT_SPLITTER_SASH_POS_CHANGING, &wxDockingFrame::OnSashPosChanging, this);

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

void wxDockingFrame::SetActivePanel(wxDockingPanel *panel)
{
	m_activePanel = panel;
}

bool wxDockingFrame::isDockable(wxWindow *window, bool *notebook) const
{
	if (notebook)
		*notebook = false;

	if (!window)
		return false;

	wxSplitterWindow *s = dynamic_cast<wxSplitterWindow*>(window);
	if (s)
		return true;

	wxNotebook *p = dynamic_cast<wxNotebook *>(window);
	if (p)
	{
		if (notebook)
			*notebook = true;

		return true;
	}

	return false;
}

wxDockingFrame *wxDockingFrame::FindDockingFrame(wxWindow *window)
{
	while (window)
	{
		wxDockingFrame *w = dynamic_cast<wxDockingFrame *>(window);
		if (w)
			return w;

		window = window->GetParent();
	}

	return nullptr;
}

wxDockingPanel *wxDockingFrame::FindDockingPanel(wxWindow *window, wxWindow **dockingChild, bool *notebook) const
{
	if (dockingChild)
		*dockingChild = window;

	while (window)
	{
		if (isDockable(window, notebook))
			return window;

		if (dockingChild)
			*dockingChild = window;

		window = window->GetParent();
	}

	if (dockingChild)
		*dockingChild = nullptr;

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

	wxSplitterWindow *splitter = CreateSplitter(parent, wxID_ANY, wxDefaultPosition, wxSize(1, 1));
	wxWindow *dummy = nullptr;

	splitter->SetSashGravity(1.0);
	dockingTarget->Reparent(splitter);
	if (dockingTarget == m_rootPanel)
		m_rootPanel = splitter;

	wxDockingPanel *dp1 = dockingTarget;
	wxDockingPanel *dp2 = userWindow;
	wxSplitterWindow *dockingTargetSplitter = dynamic_cast<wxSplitterWindow *>(dockingTarget);
	if (dockingTargetSplitter)
	{
		dummy = new wxWindow(splitter, wxID_ANY);
		dp1 = dummy;
	}

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
	int sashPos = -1;
	wxSplitterWindow *dockingSplitter = dynamic_cast<wxSplitterWindow *>(dockingTarget);

	static const int defaultWidth = 30;

	if (direction == wxLEFT || direction == wxRIGHT)
	{
		// Get the width the user desired
		sashPos = sz.x;
		sz = dockingTarget->GetSize();
		if (sashPos == -1)
			sashPos = sz.x/2;

		if (direction == wxRIGHT)
			sashPos = sz.x - sashPos;

		splitter->SplitVertically(dp1, dp2, 0);
	}
	else
	{
		sashPos = sz.y;
		sz = dockingTarget->GetSize();
		if (sashPos == -1)
			sashPos = sz.y/2;

		if (direction == wxDOWN)
			sashPos = sz.y - sashPos;

		splitter->SplitHorizontally(dp1, dp2, 0);
	}
	splitter->SetSashPosition(sashPos);
	splitter->Refresh();

	// If the parent was a splitter, we have to readjust the sashposition.
	if (dummy)
	{
		int childSashPos = dockingTargetSplitter->GetSashPosition();

		wxSize szSplitter = dockingTargetSplitter->GetClientSize();
		sz = dummy->GetClientSize();

		int orientation = dockingTargetSplitter->GetSplitMode();
		if (orientation == wxHORIZONTAL)
		{
			float sash = (float)sashPos / (float)szSplitter.y;
			childSashPos = sash * sz.y;
		}
		else
		{
			float sash = (float)sashPos / (float)szSplitter.x;
			childSashPos = sash * sz.x;
		}
		dockingTargetSplitter->SetSashPosition(childSashPos);
		childSashPos = dockingTargetSplitter->GetSashPosition();
		::gSashPos = 1;
		splitter->ReplaceWindow(dummy, dockingTarget);
		::gSashPos = 0;
		childSashPos = dockingTargetSplitter->GetSashPosition();
		delete dummy;
	}

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

	wxNotebook*nb = dynamic_cast<wxNotebook *>(parent);
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

void wxDockingFrame::OnSplitterDClick(wxSplitterEvent &event)
{
	event.Veto();
}

void wxDockingFrame::OnSashPosChanged(wxSplitterEvent &event)
{
	wxSplitterWindow *sp = dynamic_cast<wxSplitterWindow *>(event.GetEventObject());

	int n = event.GetSashPosition();
	int p = sp->GetSashPosition();
	if (gSashPos)
		event.Veto();
	else
		event.Skip();
}

void wxDockingFrame::OnSashPosChanging(wxSplitterEvent &event)
{
	wxSplitterWindow *sp = dynamic_cast<wxSplitterWindow *>(event.GetEventObject());

	int n = event.GetSashPosition();
	int p = sp->GetSashPosition();
	event.Skip();
}

void wxDockingFrame::ShowSelectorOverlay(wxRect const &window, bool allowed)
{
	if (!m_selector)
		m_selector = new wxDockingSelector(this);

	m_selector->Show();
	m_selector->Update(window, allowed);
}

void wxDockingFrame::HideSelectorOverlay(bool del)
{
	if (!m_selector)
		return;

	if (del)
	{
		delete m_selector;
		m_selector = nullptr;
	}
	else
		m_selector->Hide();
}

bool wxDockingFrame::CheckNotebook(wxPoint const &mousePos, wxDockingSpace &client)
{
	client.setPageIndex(-1);

	if (!client.isNotebook())
		return false;

	wxNotebook *nb = dynamic_cast<wxNotebook*>(client.getPanel());
	if (!nb)
		return false;

	long flags = 0;
	wxPoint point = nb->ScreenToClient(mousePos);

	// the page index can be -1. This means that the user clicked in the open area
	// near the tabs, otherwise we would have gotten a different window.
	int tabIndex = nb->HitTest(point, &flags);

	// If the user clicked "nowhwere", he clicked on the open area near the tabs, but
	// we don't get a mousevent for this. Seems that somebody is eating this before us.
	if (!(flags & wxBK_HITTEST_ONPAGE || flags & wxBK_HITTEST_NOWHERE))
	{
		client.setPageIndex(tabIndex);

		return true;
	}

	return false;
}

void wxDockingFrame::OnMouseLeftDown(wxMouseEvent& event)
{
	wxPoint mousePos = ::wxGetMousePosition();
	wxWindow *w = wxFindWindowAtPoint(mousePos);
	wxDockingPanel *p = FindDockingPanel(w);
	SetActivePanel(p);

	event.Skip();
}

void wxDockingFrame::OnMouseLeftUp(wxMouseEvent &event)
{
	wxString s;
	s
		<< "Up - "
		<< "Active: " << (void*)m_activePanel << " "
		<< "Window: " << (void*)m_event.getTarget().getWindow() << " "
		<< "Target: " << (void*)m_event.getTarget().getPanel() << " "
	;

	SetStatusText(s);
	if (m_mouseCaptured)
	{
		ReleaseMouse();
		m_mouseCaptured = false;
		HideSelectorOverlay(true);

		wxWindow *w = m_event.getSource().getWindow();
		if (w)
		{
			RemovePanel(w);
			wxDockingInfo info;
			info.dock(m_event.getTarget().getPanel());
			info.direction(m_event.getTarget().getDirection());
			wxSize sz(20, 20);
			info.size(sz);
			SplitPanel(w, info);
		}
	}

	event.Skip();
}

bool wxDockingFrame::StartEvent(wxDockingSpace &client, wxPoint const &mousePos)
{
	RecordEvent(client, mousePos);

	wxWindow *dockingSource = client.getWindow();
	if (CheckNotebook(mousePos, client) || (mousePos.y - dockingSource->GetScreenPosition().y) <= (int)m_dockingThreshold)
		return true;

	return false;
}

void wxDockingFrame::RecordEvent(wxDockingSpace &client, wxPoint const &mousePos)
{
	client.clear();

	bool isNotebook = false;
	wxWindow *dockingSource = nullptr;

	// We are using our own FindWindowAtPoint here because we need to skip our selector window.
	// When this window is shown, it would be reported as the current window at the mouse position
	// which would always be true, and we wouldn't know when a different client window is selected.
	// It also would result in flickering, because when the window is shown, the mousecursor is
	// almost always not in a position we would consider as dockable, so we switch it off. This
	// will let the underlying window report the docking position again, which means the overlay
	// is turned immediatly own again, so it constantly is switched on and off. :)
	//
	// Actually, this would also work if we pass in a nullpointer instead of selector, because
	// the overlay is a seperate frame window, which is not a child of our frame, but this way
	// it makes the code more understandable and is more resistent if this might be changed for
	// some reason.
	wxWindow *w = ::FindWindowAtPoint(this, mousePos);
	wxDockingPanel *p = FindDockingPanel(w, &dockingSource, &isNotebook);

	client.setFrame(FindDockingFrame(w));
	client.setPanel(p);
	client.setWindow(dockingSource);
	client.setNotebook(isNotebook);
}

void wxDockingFrame::OnMouseMove(wxMouseEvent &event)
{
	if (!(event.Dragging() && event.ButtonIsDown(wxMOUSE_BTN_LEFT)))
	{
		event.Skip();
		return;
	}

	wxPoint mousePos = ::wxGetMousePosition();
	if (!m_mouseCaptured)
	{
		if (!StartEvent(m_event.getSource(), mousePos))
		{
			event.Skip();
			return;
		}

		CaptureMouse();
		m_mouseCaptured = true;

		m_event.getTarget() = m_event.getSource();
		m_lastTarget.clear();
	}
	else
	{
		m_lastTarget = m_event.getTarget();
		RecordEvent(m_event.getTarget(), mousePos);
	}

	m_event.getTarget().getWindow();

	wxWindow *w = m_event.getTarget().getWindow();

	bool doPaint = false;

	// If the frame has a statusbar it wont register as a docking target.
	// However, if the user drags the mouse down there, it actually means that
	// it the window should be docked to the border.
	// TODO: Are there platforms where the statusbar is NOT at the bottom of the frame?
	int statusBarAdjustment = 0;

	bool allowed = true;
	bool toBorder = false;

	// If the pointer is outside then the user targets the border
	if (!w)
	{
		toBorder = true;
		w = this;		// Target is the border
	}

	wxPoint clPos = w->ScreenToClient(mousePos);
	wxRect wr = w->GetScreenRect();
	wxRect cr = w->GetClientRect();
	w->ClientToScreen(&cr.x, &cr.y);

	wxSize border = w->GetWindowBorderSize();
	wxDirection direction = wxCENTRAL;

	wxString s;
	s
		<< "Move - "
		<< "Window: " << (void *)m_event.getTarget().getWindow() << " "
		<< "Target: " << (void *)m_event.getTarget().getPanel() << " "
		<< "MousePos: " << mousePos.x << "/" << mousePos.y << " "
		<< "Area: " << wr.x << "/" << wr.y << "/" << wr.width << "/" << wr.height << " "
	;
	SetStatusText(s);

	if (mousePos.y < (cr.y+(int)m_dockingWidth))
	{
		// Cursor is near the top border
		direction = wxUP;
		wr.height = (int)m_dockingWidth;
		doPaint = true;
	}
	else if (mousePos.y > (cr.y + (cr.height - (int)m_dockingWidth) - statusBarAdjustment))
	{
		// Cursor is at the bottom border
		direction = wxDOWN;
		wr.height -= (int)m_dockingWidth;
		wr.y += wr.height;
		wr.height = (int)m_dockingWidth;;
		doPaint = true;
	}
	else if (clPos.x < (int)m_dockingWidth)
	{
		// Cursor is at the left border
		direction = wxLEFT;
		wr.width = (int)m_dockingWidth;
		doPaint = true;
	}
	else if (clPos.x > (cr.width - (int)m_dockingWidth))
	{
		// Cursor is at the right border
		direction = wxRIGHT;
		wr.width -= (int)m_dockingWidth;
		wr.x += wr.width;
		wr.width = (int)m_dockingWidth;;
		doPaint = true;
	}

	if (doPaint)
	{
		// In Windows 10 the frame window has a shadow around it. This is included
		// as part of the window size, but for the user it doesn't look like it. So
		// the overlay appears to be slightly to big/shifted in the wrong position
		// and we have to adjust for it. This may not be needed on other platforms
		// or might be handled differently there.
		if (w == this)
		{
			if (direction == wxUP || direction == wxDOWN)
			{
				wr.width -= (border.x * 2);
				wr.x += border.x;
			}
			else if (direction == wxLEFT || direction == wxRIGHT)
				wr.height -= (border.y * 2);
		}

		if (m_event.getTarget().getWindow() == m_event.getSource().getWindow())
			allowed = false;

		m_event.getTarget().setDirection(direction);
		ShowSelectorOverlay(wr, allowed);
	}
	else
		HideSelectorOverlay();

	//event.Skip();
}

#endif // wxUSE_DOCKING
