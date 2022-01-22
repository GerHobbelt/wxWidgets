#if wxUSE_DOCKING

#include <wx/docking/dockinginfo.h>
#include <wx/notebook.h>

wxDockingInfo::wxDockingInfo(wxString const &title)
: m_title(title)
, m_dockingPanel(nullptr)
, m_direction(wxCENTRAL)
, m_nbTabStyle(wxCENTRAL)
, m_horizontal(true)
, m_point(wxDefaultPosition)
, m_size(wxSize(-1, -1))
, m_activate(false)
{
	// Override default settings if available
	if (m_default)
	{
		m_nbTabStyle = m_default->m_nbTabStyle;
	}
}

long wxDockingInfo::tabStyle(void) const
{
	switch (m_nbTabStyle)
	{
		case wxLEFT: return wxNB_LEFT;
		case wxRIGHT: return wxNB_RIGHT;
		case wxTOP: return wxNB_TOP;
		case wxBOTTOM: return wxNB_BOTTOM;
	}

	return 0;
}

#endif // wxUSE_DOCKING
