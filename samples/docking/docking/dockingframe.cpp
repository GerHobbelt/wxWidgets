// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#if wxUSE_DOCKING

#include <wx/app.h>
#include <wx/object.h>
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

wxDockingInfo *wxDockingInfo::m_default = NULL;
class wxDockingEventFilter;

// If a panel is floating, we have to keep track of it
// so we can properly de-/serialize the layout.
// this list will contain only floating frames.
static vector<wxDockingFrame *> gFrames;
static wxDockingEventFilter *gEventFilter;

class wxDockingEventFilter : public wxEventFilter
{
public:
	wxDockingEventFilter()
	{
	}

	~wxDockingEventFilter() wxOVERRIDE
	{
	}

	int FilterEvent(wxEvent &event) wxOVERRIDE
	{
		// We want to be sure that the frame for the given window is used. The client can
		// have multiple frame windows (for example when a window is floating), so we have
		// to make sure that the event is propagated to the associated frame.
		const wxEventType t = event.GetEventType();
		if (t == wxEVT_LEFT_DOWN)
		{
			wxWindow *w = wxDynamicCast(event.GetEventObject(), wxWindow);
			wxDockingFrame *frame = wxDockingFrameFromWindow(w);
			if (!frame)
				return -1;

			return frame->OnMouseLeftDown((wxMouseEvent &)event);
		}

		if (t == wxEVT_LEFT_UP)
		{
			wxWindow *w = wxDynamicCast(event.GetEventObject(), wxWindow);
			wxDockingFrame *frame = wxDockingFrameFromWindow(w);
			if (!frame)
				return -1;

			return frame->OnMouseLeftUp((wxMouseEvent &)event);
		}

		if (t == wxEVT_MOTION)
		{
			wxWindow *w = wxDynamicCast(event.GetEventObject(), wxWindow);
			wxDockingFrame *frame = wxDockingFrameFromWindow(w);
			if (!frame)
				return -1;

			return frame->OnMouseMove((wxMouseEvent &)event);
		}

		return -1;	// Continue regular processing
	}
};

// ------------------------------------------------------------------------------------------
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

	if (gFrames.empty())
	{
		delete gEventFilter;
		gEventFilter = NULL;
	}
}

void wxDockingFrame::init()
{
	m_rootPanel = NULL;
	m_sizer = NULL;

	m_toolbarsLeft = NULL;
	m_toolbarsRight = NULL;
	m_toolbarsTop = NULL;
	m_toolbarsBottom = NULL;
	m_selector = NULL;

	m_mouseCaptured = false;

	m_dockingWidth = 30;

	m_event.Reset();
	m_defaults.SetDefaults();
}

bool wxDockingFrame::Create(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
{
	m_activePanel = NULL;

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

void wxDockingFrame::BindEventHandlers()
{
	// We only bind our event handlers for the main frame.
	if (gFrames[0] != this)
		return;

	wxApp *app = static_cast<wxApp *>(wxApp::GetInstance());
	if (!gEventFilter)
		gEventFilter = new wxDockingEventFilter();

	app->AddFilter(gEventFilter);

	//app->Bind(wxEVT_LEFT_DOWN, &wxDockingFrame::OnMouseLeftDown, this);
	//app->Bind(wxEVT_LEFT_UP, &wxDockingFrame::OnMouseLeftUp, this);
	//app->Bind(wxEVT_MOTION, &wxDockingFrame::OnMouseMove, this);

	app->Bind(wxEVT_SPLITTER_SASH_POS_RESIZE, &wxDockingFrame::OnSplitterSashUpdate, this);
	app->Bind(wxEVT_SPLITTER_DOUBLECLICKED, &wxDockingFrame::OnSplitterDClick, this);

	app->Bind(wxEVT_SIZE, &wxDockingFrame::OnSize, this);
	app->Bind(wxEVT_SIZING, &wxDockingFrame::OnSize, this);
}

void wxDockingFrame::UnbindEventHandlers()
{
	if (gFrames[0] != this)
		return;

	wxApp *app = static_cast<wxApp *>(wxApp::GetInstance());

//	app->Unbind(wxEVT_LEFT_DOWN, &wxDockingFrame::OnMouseLeftDown, this);
//	app->Unbind(wxEVT_LEFT_UP, &wxDockingFrame::OnMouseLeftUp, this);
//	app->Unbind(wxEVT_MOTION, &wxDockingFrame::OnMouseMove, this);

	app->Unbind(wxEVT_SPLITTER_SASH_POS_RESIZE, &wxDockingFrame::OnSplitterSashUpdate, this);
	app->Unbind(wxEVT_SPLITTER_DOUBLECLICKED, &wxDockingFrame::OnSplitterDClick, this);

	app->Unbind(wxEVT_SIZE, &wxDockingFrame::OnSize, this);
	app->Unbind(wxEVT_SIZING, &wxDockingFrame::OnSize, this);

	app->RemoveFilter(gEventFilter);
}

void wxDockingFrame::OnSize(wxSizeEvent &event)
{
	DoSize();
	event.Skip();
}

void wxDockingFrame::DoSize()
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

bool wxDockingFrame::CanDock(wxWindow *window, wxDockingPanelType *panelType)
{
	if (panelType)
		*panelType = wxDOCKING_NONE;

	if (!window)
		return false;

	wxSplitterWindow *s = wxDynamicCast(window, wxSplitterWindow);
	if (s)
	{
		if (panelType)
			*panelType = wxDOCKING_SPLITTER;

		return true;
	}

	wxNotebook *p = wxDynamicCast(window, wxNotebook);
	if (p)
	{
		if (panelType)
			*panelType = wxDOCKING_NOTEBOOK;

		return true;
	}

	return false;
}

bool wxDockingFrame::isDockable(wxWindow *window, wxDockingPanelType  *panelType)
{
	if (CanDock(window, panelType))
		return true;

	if (!window)
		return false;

	// If the window itself was not a dockable, we check if the parent is a dockable.
	// we can dock to any window which is a splitter, or a notebook or a direct
	// descendant of it.
	window = window->GetParent();
	return CanDock(window, NULL);
}

wxDockingPanel *wxDockingFrame::FloatPanel(wxWindow *panel, wxDockingInfo const &info)
{
	wxDockingPanel *dp = NULL;

/*	long style = wxSYSTEM_MENU | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX | wxCAPTION | wxCLIP_CHILDREN;
	wxDockingFrame *frame = new wxDockingFrame(this, wxID_ANY, info.title(), info.position(), info.size(), style);
	wxDockingPanel *dp = frame->GetRootPanel();
	dp->SetTitle(info.title());
	panel->Reparent(dp);
	dp->SetFloatingWindow(frame);
	frame->Show();*/

	return dp;
}

/*
wxString wxDockingFrame::SerializeLayout() const
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

wxString wxDockingFrame::SerializeFrame() const
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

void wxDockingFrame::UpdateToolbarLayout()
{
	SetSizer(NULL, true);

/*	m_sizer = new wxGridBagSizer(0, 0);
	m_sizer->SetFlexibleDirection(wxBOTH);
	m_sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	int top = m_toolbarsTop != NULL;
	int left = m_toolbarsLeft != NULL;
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
		return NULL;

	wxDockingPanel *dp = NULL;

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
*/
	UpdateToolbarLayout();

	return dp;
}

bool wxDockingFrame::HideToolbar(wxDockingPanel *&toolbar)
{
	if (toolbar)
	{
		RemoveChild(toolbar);
		delete toolbar;
		toolbar = NULL;
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
	wxSplitterWindow *splitter = new wxSplitterWindow(parent, id, pos, size, style);
	splitter->SetSashGravity(0.5f);
	splitter->SetProportionalSash(true);

	return splitter;
}

void wxDockingFrame::DeleteSplitter(wxSplitterWindow *splitter)
{
	delete splitter;
}

wxDockingPanel *wxDockingFrame::CreateTabPanel(wxWindow *userWindow, wxDockingInfo const &info, wxWindow *parent)
{
	wxCHECK_MSG(userWindow, NULL, wxT("userWindow is a NULL"));

	wxDockingPanel *dockingTarget = info.GetPanel();

	if (!dockingTarget)
		dockingTarget = userWindow->GetParent();
	else
	{
		wxCHECK_MSG(isDockable(dockingTarget), NULL, wxT("Docking target is not dockable for tab"));
	}

	if (!parent)
		parent = dockingTarget->GetParent();

	if (!parent)
		parent = dockingTarget;

	// If the docking target itself is a notebook we just add the new window as a page to it.
	wxNotebook *nb = wxDynamicCast(dockingTarget, wxNotebook);
	if (!nb)
	{
		// If the dockingtarget is a page of a notebook, we have to remove that page
		// and replace it with a new notebook where that page is added to.
		nb = CreateNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, info.GetTabStyle());
		wxString title;

		wxNotebook *pnb = wxDynamicCast(parent, wxNotebook);
		if (pnb)
		{
			int pageIndex = -1;

			pageIndex = pnb->FindPage(dockingTarget);
			if (pageIndex == wxNOT_FOUND)
				return NULL;

			title = pnb->GetPageText(pageIndex);
			pnb->RemovePage(pageIndex);
			pnb->InsertPage(pageIndex, nb, title, true);
			dockingTarget->Reparent(nb);
			nb->AddPage(dockingTarget, title, true);
		}
		else
		{
			wxSplitterWindow *parentSplitter = wxDynamicCast(parent, wxSplitterWindow);
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
			nb = CreateNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, info.GetTabStyle());
	}

	userWindow->Reparent(nb);
	nb->AddPage(userWindow, info.GetTitle(), true);

	if (!m_rootPanel)
		m_rootPanel = nb;

	return nb;
}

wxDockingPanel *wxDockingFrame::AddTabPanel(wxWindow *userWindow, wxDockingInfo const &info)
{
	wxDockingPanel *dp = CreateTabPanel(userWindow, info, NULL);

	if (dp)
		SetActivePanel(dp);

	return dp;
}

wxNotebook *wxDockingFrame::ReplaceNotebookPage(wxNotebook *notebook, wxWindow *oldPage, int &index, wxDockingInfo const &info)
{
	if (!notebook)
	{
		index = wxNOT_FOUND;
		return NULL;
	}

	index = notebook->FindPage(oldPage);
	if (index == wxNOT_FOUND)
		return NULL;

	wxString title = notebook->GetPageText(index);
	wxNotebook *nb = CreateNotebook(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, info.GetTabStyle());
	notebook->RemovePage(index);
	oldPage->Reparent(nb);
	notebook->InsertPage(index, nb, title, true);
	nb->AddPage(oldPage, title, true);

	return nb;
}

wxDockingPanel *wxDockingFrame::AddPanel(wxWindow *window, wxDockingInfo const &info)
{
	return nullptr;
}

wxDockingPanel *wxDockingFrame::SplitPanel(wxWindow *userWindow, wxDockingInfo const &info)
{
	wxDockingPanel *dockingTarget = info.GetPanel();
	bool forceResize = false;

	if (!dockingTarget)
	{
		dockingTarget = m_rootPanel;
		forceResize = true;
	}

	wxCHECK_MSG(isDockable(dockingTarget), NULL, wxT("Docking target is not dockable for splitter"));

	wxWindow *parent = dockingTarget->GetParent();

	wxSplitterWindow *splitter = CreateSplitter(parent, wxID_ANY, wxDefaultPosition, wxSize(1, 1));
	splitter->SetProportionalSash(true);

	wxWindow *dummy = NULL;

	splitter->SetSashGravity(1.0);
	dockingTarget->Reparent(splitter);
	if (dockingTarget == m_rootPanel)
		m_rootPanel = splitter;

	wxDockingPanel *dp1 = dockingTarget;
	wxDockingPanel *dp2 = userWindow;

	// If the docking target is a splitter itself, we create a dummy window for insertion, so the
	// sashpositions wont get screwed up during the insert process.
	wxSplitterWindow *dockingTargetSplitter = wxDynamicCast(dockingTarget, wxSplitterWindow);
	if (dockingTargetSplitter)
	{
		dummy = new wxWindow(splitter, wxID_ANY);
		dp1 = dummy;
	}

	// When the parent is a docking window, we have to remove the panel from it and replace it
	// with our new splitter window. The taken panel becomes part of the splitter then.
	wxSplitterWindow *parentSplitter = wxDynamicCast(parent, wxSplitterWindow);
	if (parentSplitter)
		parentSplitter->ReplaceWindow(dockingTarget, splitter);
	else
	{
		wxNotebook *pnb = wxDynamicCast(parent, wxNotebook);
		if (pnb)
		{
			int pageIndex = -1;

			pageIndex = pnb->FindPage(dockingTarget);
			if (pageIndex == wxNOT_FOUND)
				return NULL;

			wxString title = pnb->GetPageText(pageIndex);
			wxNotebook *nb = CreateNotebook(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, info.GetTabStyle());
			pnb->RemovePage(pageIndex);
			dockingTarget->Reparent(nb);
			pnb->InsertPage(pageIndex, splitter, title, true);
			nb->AddPage(dockingTarget, title, true);
			dp1 = nb;
		}
	}

	wxDockingInfo childInfo = info;
	childInfo.SetPanel(splitter);
	childInfo.activate();
	dp2->Reparent(splitter);
	wxSize sz = info.GetSize();

	wxDirection direction = info.GetDirection();
	if (direction == wxLEFT || direction == wxUP)
		swap(dp1, dp2);

	int sashPos = -1;
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

	// The sash position is not properly updated when calling SplitHoriz/Vertically
	// so we have to do it manually afterwards.
	splitter->SetSashPosition(sashPos);

	if (dummy)
	{
		splitter->ReplaceWindow(dummy, dockingTarget);
		delete dummy;
	}

	SetActivePanel(splitter);

	return splitter;
}

wxDockingPanel *wxDockingFrame::RemovePanel(wxDockingInfo &info)
{
	wxDockingFrame *frame = info.GetFrame();
	if (!frame)
		frame = this;

	return frame->RemovePanel(info.GetWindow());
}

wxDockingPanel *wxDockingFrame::RemovePanel(wxWindow *userWindow)
{
	if (!userWindow)
		return NULL;

	wxWindow *parent = userWindow->GetParent();
	if (parent == NULL)
		return NULL;

	wxWindow *newDockingTarget = parent->GetParent();
	if (!newDockingTarget)
		newDockingTarget = this;

	userWindow->Reparent(this);

	wxNotebook *nb = wxDynamicCast(parent, wxNotebook);
	if (nb)
	{
		wxDockingPanel *dp = RemoveFromNotebook(userWindow, nb);
		SetActivePanel(dp);

		return dp;
	}

	wxSplitterWindow *splitter = wxDynamicCast(parent, wxSplitterWindow);
	if (splitter)
	{
		wxDockingPanel *dp = RemoveFromSplitter(parent, userWindow, splitter);
		SetActivePanel(dp);

		return dp;
	}

	return NULL;
}

wxDockingPanel *wxDockingFrame::RemoveFromSplitter(wxWindow *parent, wxWindow *window, wxSplitterWindow *splitter)
{
	wxWindow *newDockingTarget = parent->GetParent();
	if (!newDockingTarget)
		newDockingTarget = this;

	wxWindow *otherWindow = splitter->GetWindow1();
	if (otherWindow == window)
		otherWindow = splitter->GetWindow2();

	splitter->Unsplit(window);
	splitter->Unsplit(otherWindow);
	otherWindow->Reparent(newDockingTarget);

	wxSplitterWindow *parentSplitter = wxDynamicCast(splitter->GetParent(), wxSplitterWindow);
	if (parentSplitter)
		parentSplitter->ReplaceWindow(splitter, otherWindow);

	DeleteSplitter(splitter);

	wxDockingFrame *frame = wxDynamicCast(newDockingTarget, wxDockingFrame);
	if (frame)
	{
		if (frame->m_rootPanel == splitter)
			frame->m_rootPanel = otherWindow;
	}

	return newDockingTarget;
}

wxDockingPanel *wxDockingFrame::RemoveFromNotebook(wxWindow *page, wxNotebook *notebook)
{
	int pageIndex = -1;

	pageIndex = notebook->FindPage(page);
	if (pageIndex == wxNOT_FOUND)
		return NULL;

	notebook->RemovePage(pageIndex);

	return notebook;
}

bool wxDockingFrame::MovePanel(wxWindow *sourceWindow, wxDockingInfo &tgt)
{
	wxDockingInfo src;
	if (!src.CollectInfo(sourceWindow))
		return false;

	return MovePanel(src, tgt);
}

bool wxDockingFrame::MovePanel(wxDockingInfo &src, wxDockingInfo &tgt)
{
	// TODO: If the target is the source window we can do two things.
	// 1. Cancel the docking because it doesn't make sense to dock on itself.
	// 2. Make the window floating. If not, then the user would have to drag
	// the window to the border which could be annoying. On the other hand
	// I always hated it that it sometimes happens that I accidently move the
	// mouse and the window becomes floated, so option 1 would be nice to have.
	// An option could be to provide a cancel overlay. We may need an overlay
	// GUI anyway to distinguish between floating, border or docking to the a
	// window which happens to be on one of the borders. Especially for wxNotebook
	// docking this will be needed as it can not be guessed automatically where the
	// user really wants to connect. To the notebook as a page, to the left/right/etc.
	// of the notebook or to the border where a notebook happens to be in the way.
	wxWindow *w = src.GetWindow();
	if (tgt.GetWindow() != w && w)	// For now we cancel if docking to itself.
	{
		wxDockingPanel *tp = tgt.GetPanel();
		if (tp == src.GetPanel())
		{
			if (tgt.GetPanelType() == wxDOCKING_SPLITTER)
				return DoMoveSplitter(src, tgt);
		}
		else
		{
			wxDockingPanel *p = RemovePanel(src);

			// Our new docking target is the window the user is currently pointing to.
			tgt.SetPanel(tgt.GetWindow());
			p = SplitPanel(w, tgt);
		}
	}

	return true;
}

bool wxDockingFrame::DoMoveSplitter(wxDockingInfo &src, wxDockingInfo &tgt)
{
	wxDockingPanel *tp = tgt.GetPanel();
	wxSplitterWindow *sp = wxDynamicCast(tp, wxSplitterWindow);

	// Avoid jittering
	sp->Freeze();

	wxOrientation orientation = src.GetOrientation();
	wxDirection td = tgt.GetDirection();

	if (
		((td == wxLEFT || td == wxRIGHT) && orientation == wxHORIZONTAL)
		|| ((td == wxTOP || td == wxBOTTOM) && orientation == wxVERTICAL)
		)
	{
		// We have to change the orientation
		if (orientation == wxHORIZONTAL)
			sp->SetSplitMode(wxSPLIT_VERTICAL);
		else
			sp->SetSplitMode(wxSPLIT_HORIZONTAL);

		// TODO: Is there a better way to enforce a refresh? Refresh/Update doesn't work.
		//sp->Refresh();
		//sp->Update();
		wxSize sz = sp->GetSize();
		sz.x--;
		sp->SetSize(sz);
		sz.x++;
		sp->SetSize(sz);
	}

	// Check if the new direction is the same as before. If not, we have to switch the windows
	wxDirection sd = src.GetDirection();
	bool sw1 = (sd == wxLEFT || sd == wxTOP) ? true : false;
	bool sw2 = (td == wxLEFT || td == wxTOP) ? true : false;
	if (sw1 != sw2)
	{
		wxWindow *w1 = sp->GetWindow1();
		wxWindow *w2 = sp->GetWindow2();
		sp->ReplaceWindow(w1, w2);
		sp->ReplaceWindow(w2, w1);
	}

	sp->Thaw();

	return true;
}

void wxDockingFrame::OnSplitterSashUpdate(wxSplitterEvent &event)
{
	wxSplitterWindow *splitter = wxDynamicCast(event.GetEventObject(), wxSplitterWindow);

	int oldPos = splitter->GetSashPosition();
	if (oldPos == 0)
	{
		event.Skip();

		return;
	}

	// If the sash should be kept at the same relative position as it was
	// before the resize, we have to calculate the new position based on the ratio.
	if (oldPos)
	{
		int oldSize = event.GetOldSize();
		int newSize = event.GetNewSize();

		float ratio = (float)oldPos / (float)oldSize;
		int pos = newSize * ratio;
		event.SetSashPosition(pos);
	}

	event.Skip();
}

void wxDockingFrame::OnSplitterDClick(wxSplitterEvent &event)
{
	// Disable the double click on a splitter. The default behavior is
	// to remove the sash, whcih we don't want.
	// TODO: Maybe we should check if the double click was really on the
	// sash, as there might be other usecases wanting to capture double clicks
	// as well.
	event.Veto();
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
		m_selector = NULL;
	}
	else
		m_selector->Hide();
}

bool wxDockingFrame::CheckNotebook(wxPoint const &mousePos, wxDockingInfo &info)
{
	info.SetPage(-1);

	if (info.GetPanelType() != wxDOCKING_NOTEBOOK)
		return false;

	wxNotebook *nb = wxDynamicCast(info.GetPanel(), wxNotebook);
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
		info.SetPage(tabIndex);

		return true;
	}

	return false;
}

int wxDockingFrame::OnMouseLeftDown(wxMouseEvent &event)
{
	wxPoint mousePos = ::wxGetMousePosition();
	wxWindow *w = wxFindWindowAtPoint(mousePos);
	wxWindow *dockingChild;
	wxDockingPanel *p = wxDockingFindPanel(w, &dockingChild);
	SetActivePanel(p);

	wxString s;
	s
		<< "LeftDown - "
		<< "Panel: " << (void *)p << " "
		<< "Window: " << (void *)dockingChild << " "
	;
	SetStatusText(s);

	event.Skip();
	return -1;
}

int wxDockingFrame::OnMouseLeftUp(wxMouseEvent &WXUNUSED(event))
{
	if (m_mouseCaptured)
	{
		wxDockingPanelPtr sptr(m_event.GetSource().GetWindow());
		wxDockingPanelPtr tptr(m_event.GetTarget().GetWindow());

		wxString s;
		s
			<< "Up - "
			<< "Panel: " << (void *)m_event.GetSource().GetPanel() << " "
			<< "Window: " << (void *)m_event.GetSource().GetWindow() << " "
			<< "Type: " << sptr.GetType() << " "
			<< " ===> "
			<< "Panel: " << (void *)m_event.GetTarget().GetPanel() << " "
			<< "Window: " << (void *)m_event.GetTarget().GetWindow() << " "
			<< "Type: " << tptr.GetType() << " "
			;
		SetStatusText(s);

		ReleaseMouse();
		m_mouseCaptured = false;
		HideSelectorOverlay(true);

		MovePanel(m_event.GetSource(), m_event.GetTarget());
	}

	return -1;
}

bool wxDockingFrame::InitSourceEvent(wxPoint const &mousePos)
{
	// We are using our own FindWindowAtPoint here because we need to skip our selector window.
	// When this window is shown, it would be reported as the current window at the mouse position
	// which would always be true, and we wouldn't know when a different client window is selected.
	// It also would result in flickering, because when the window is shown, the mousecursor is
	// almost always not in a position we would consider as dockable, so we switch it off. This
	// will let the underlying window report the docking position again, which means the overlay
	// is turned immediatly own again, so it is constantly switched on and off. :)

	wxDockingInfo &src = m_event.GetSource();

	wxWindow *w = wxDockingWindowAtPoint(this, mousePos);
	src.CollectInfo(w);
	if (!src.GetWindow())
		src.SetWindow(src.GetPanel());

	wxWindow *dockingSource = src.GetWindow();
	if (CheckNotebook(mousePos, src) || (mousePos.y - dockingSource->GetScreenPosition().y) <= (int)m_dockingWidth)
		return true;

	return false;
}

int wxDockingFrame::OnMouseMove(wxMouseEvent &event)
{
	if (!(event.Dragging() && event.ButtonIsDown(wxMOUSE_BTN_LEFT)))
	{
		event.Skip();
		return -1;
	}

	wxPoint mousePos = ::wxGetMousePosition();
	if (!m_mouseCaptured)
	{
		wxWindow *cw = GetCapture();

		// If the mouse is already captured by some other component, we let it pass through.
		// This happens i.E. when the splitter handle is grabbed for resizing.
		if ((cw && cw != this) || !InitSourceEvent(mousePos))
		{
			event.Skip();
			return -1;
		}

		CaptureMouse();
		m_mouseCaptured = true;

		m_event.GetTarget() = m_event.GetSource();
		m_lastTarget.Clear();
	}
	else
	{
		m_lastTarget = m_event.GetTarget();
		wxWindow *w = wxDockingWindowAtPoint(this, mousePos);
		m_event.GetTarget().CollectInfo(w);
	}

	wxDockingInfo &src = m_event.GetSource();
	wxDockingInfo &tgt = m_event.GetTarget();

	wxWindow *w = tgt.GetWindow();
	if (!w)
		w = tgt.GetPanel();

	bool showOverlay = false;
	bool allowed = true;
	bool toBorder = false;

	// If the pointer is outside then the user targets the border
	if (!w)
	{
		toBorder = true;
		w = this;		// Target is the border
	}

	wxDockingPanelPtr ptr(w);
	wxRect wr;
	wxDirection direction = wxCENTRAL;

	wxDockingPanelPtr panel = wxDockingPanelPtr(w);
	if (panel.GetType() == wxDOCKING_NOTEBOOK)
	{
/*		wxNotebook *nb = panel.GetNotebook();

		int sel = src.GetPage();
		if (sel != wxNOT_FOUND)
		{
			wr = nb->GetTabRect(sel);
			nb->ClientToScreen(&wr.x, &wr.y);
		}

		showOverlay = true;
		//allowed = true;
*/
	}
	else
	{
		showOverlay = CalcOverlayRectangle(mousePos, w, direction, wr);
		if (tgt.GetWindow() == src.GetWindow())
			allowed = false;
	}

	wxString s;
	s
		<< "Source - "
		<< "Panel: " << (void *)src.GetPanel() << " "
		<< "Window: " << (void *)src.GetWindow() << " "
		<< "Page: " << src.GetPage() << " "
		<< " ===> "
		<< "Target - "
		<< "Panel: " << (void *)tgt.GetPanel() << " "
		<< "Window: " << (void *)tgt.GetWindow() << " "
		<< "Page: " << tgt.GetPage() << " "
		<< "MousePos: " << mousePos.x << "/" << mousePos.y << " "
		<< "Area: " << wr.x << "/" << wr.y << "/" << wr.width << "/" << wr.height << " "
		<< "Type: " << ptr.GetType() << " "
		;
	SetStatusText(s);

	if (showOverlay)
	{
		tgt.SetDirection(direction);
		ShowSelectorOverlay(wr, allowed);
	}
	else
		HideSelectorOverlay();

	// TODO: Should this return false to stop processing? After all, we already processed it
	// and we don't want client code to interfere with the docking. What happens to client code
	// that also needs to track motion?
	return true;
}

bool wxDockingFrame::CalcOverlayRectangle(wxPoint const &mousePos, wxWindow *curWindow, wxDirection &direction, wxRect &wr)
{
	bool doPaint = false;
	direction = wxCENTRAL;

	wxPoint clPos = curWindow->ScreenToClient(mousePos);
	wxRect cr = curWindow->GetClientRect();
	curWindow->ClientToScreen(&cr.x, &cr.y);
	wr = curWindow->GetScreenRect();

	wxSize border = curWindow->GetWindowBorderSize();
	if (mousePos.y < (cr.y + (int)m_dockingWidth))
	{
		// Cursor is near the top border
		direction = wxUP;
		wr.height = (int)m_dockingWidth;
		doPaint = true;
	}
	else if (mousePos.y > (cr.y + (cr.height - (int)m_dockingWidth)))
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

	// TODO: In Windows 10 the frame window has a shadow around it. This is included
	// as part of the window size, but for the user it doesn't look like it. So
	// the overlay appears to be slightly to big/shifted in the wrong position
	// and we have to adjust for it. This may not be needed on other platforms
	// or might be handled differently there.
	if (curWindow == this)
	{
		if (direction == wxUP || direction == wxDOWN)
		{
			wr.width -= (border.x * 2);
			wr.x += border.x;
		}
		else if (direction == wxLEFT || direction == wxRIGHT)
			wr.height -= (border.y * 2);
	}

	return doPaint;
}

#endif // wxUSE_DOCKING
