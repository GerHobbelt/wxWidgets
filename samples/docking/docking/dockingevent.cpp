// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#if wxUSE_DOCKING

#include <wx/docking/dockingevent.h>

#if wxUSE_GUI
wxIMPLEMENT_DYNAMIC_CLASS(wxDockingEvent, wxEvent);
#endif // wxUSE_GUI

wxDockingEvent::wxDockingEvent(void)
{
}

void wxDockingEvent::Assign(const wxDockingEvent &event)
{
	wxEvent::operator=(event);

	m_src = event.m_src;
	m_tgt = event.m_tgt;
}

#endif // wxUSE_DOCKING
