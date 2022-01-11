#pragma once

#include "wx/defs.h"

#if wxUSE_DOCKING

#include <wx/docking/docking_defs.h>
#include <wx/docking/dockingspace.h>

#include <wx/notebook.h>
#include <wx/splitter.h>

class WXDLLIMPEXP_CORE wxDockingEvent : public wxEvent
{
public:
	wxDockingEvent(void);
	wxDockingEvent(const wxDockingEvent &event)
		: wxEvent(event)
	{
		Assign(event);
	}

	wxDockingEvent &operator=(const wxDockingEvent &event)
	{
		if (&event != this)
			Assign(event);

		return *this;
	}

	void clear(void)
	{
		Assign(wxDockingEvent());
	}

	void setSource(wxDockingSpace &site) { m_src = site; }
	const wxDockingSpace &getSource(void) const { return m_src; }
	wxDockingSpace &getSource(void) { return m_src; }

	void setTarget(wxDockingSpace &site) { m_tgt = site; }
	const wxDockingSpace &getTarget(void) const { return m_tgt; }
	wxDockingSpace& getTarget(void) { return m_tgt; }

	virtual wxEvent *Clone() const wxOVERRIDE { return new wxDockingEvent(*this); }
	virtual wxEventCategory GetEventCategory() const wxOVERRIDE { return wxEVT_CATEGORY_USER_INPUT; }

protected:
	void Assign(const wxDockingEvent &evt);

private:
	wxDockingSpace m_src;
	wxDockingSpace m_tgt;

	wxDECLARE_DYNAMIC_CLASS(wxDockingEvent);
};

#endif // wxUSE_DOCKING
