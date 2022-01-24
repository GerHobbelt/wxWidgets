// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#if wxUSE_DOCKING

#include <wx/object.h>
#include <wx/notebook.h>
#include <wx/splitter.h>

#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingutils.h>

wxDockingInfo::wxDockingInfo(wxString const &title)
: m_title(title)
, m_frame(NULL)
, m_panel(NULL)
, m_dockingPanelType(wxDOCKING_NONE)
, m_direction(wxCENTRAL)
, m_tabDirection(wxTOP)
, m_orientation((wxOrientation)0)
, m_size(wxSize(-1, -1))
, m_activate(false)
, m_page(-1)
{
	// Override default settings if available
	if (m_default)
	{
		m_tabDirection = m_default->m_tabDirection;
	}
}

void wxDockingInfo::Clear()
{
	m_title = "";
	m_frame = NULL;
	m_panel = NULL;
	m_direction = wxCENTRAL;
	m_orientation = (wxOrientation)0;
	m_dockingPanelType = wxDOCKING_NONE;
	m_position = wxPoint(-1, -1);
	m_size = wxSize(-1, -1);
	m_activate = false;
	m_tabDirection = wxTOP;
	m_page = -1;
}

long wxDockingInfo::GetTabStyle() const
{
	switch (m_tabDirection)
	{
		case wxLEFT: return wxNB_LEFT;
		case wxRIGHT: return wxNB_RIGHT;
		case wxTOP: return wxNB_TOP;
		case wxBOTTOM: return wxNB_BOTTOM;
	}

	return 0;
}

wxDockingInfo &wxDockingInfo::SetOrientation(wxOrientation orientation)
{
	if (orientation != wxHORIZONTAL && orientation != wxVERTICAL)
		orientation = (wxOrientation)0;

	m_orientation = orientation;

	return *this;
}

bool wxDockingInfo::CollectInfo(wxWindow *source)
{
	Clear();

	if (!source)
		return false;

	wxDockingPanelType panelType = wxDOCKING_NONE;
	wxWindow *dockingSource = NULL;

	// If the source window is not part of a docking panel, we can't do anything about it.
	wxDockingPanel *p = wxDockingFindPanel(source, &dockingSource, &panelType);
	if (!p)
		return false;

	SetFrame(wxDockingFrameFromWindow(source));
	SetWindow(dockingSource);
	SetPanel(p);
	SetPanelType(panelType);

	if (panelType == wxDOCKING_SPLITTER && dockingSource)
	{
		wxSplitterWindow *sp = wxDynamicCast(p, wxSplitterWindow);
		wxOrientation orientation = (sp->GetSplitMode() == wxSPLIT_HORIZONTAL) ? wxHORIZONTAL : wxVERTICAL;
		SetOrientation(orientation);

		wxWindow *w1 = sp->GetWindow1();

		if (orientation == wxVERTICAL)
		{
			if (dockingSource == w1)
				SetDirection(wxLEFT);
			else
				SetDirection(wxRIGHT);
		}
		else
		{
			if (dockingSource == w1)
				SetDirection(wxTOP);
			else
				SetDirection(wxBOTTOM);
		}
	}

	if (!dockingSource)
		dockingSource = p;


	return true;
}

#endif // wxUSE_DOCKING
