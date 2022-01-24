#ifndef _WX_DOCKING_EVENT_H_
#define _WX_DOCKING_EVENT_H_

#include "wx/defs.h"

#if wxUSE_DOCKING

#include <wx/docking/docking_defs.h>
#include <wx/docking/dockinginfo.h>

#include <wx/event.h>

class WXDLLIMPEXP_DOCKING wxDockingEvent : public wxEvent
{
public:
	wxDockingEvent();
	wxDockingEvent(const wxDockingEvent &event)
	: wxEvent(event)
	{
		Assign(event);
	}

	~wxDockingEvent() wxOVERRIDE
	{
	}

	wxDockingEvent &operator=(const wxDockingEvent &event)
	{
		if (&event != this)
			Assign(event);

		return *this;
	}

	void Reset()
	{
		Assign(wxDockingEvent());
	}

	void SetSource(wxDockingInfo &site) { m_src = site; }
	const wxDockingInfo &GetSource() const { return m_src; }
	wxDockingInfo &GetSource() { return m_src; }

	void SetTarget(wxDockingInfo &site) { m_tgt = site; }
	const wxDockingInfo &GetTarget() const { return m_tgt; }
	wxDockingInfo &GetTarget() { return m_tgt; }

	virtual wxEvent *Clone() const wxOVERRIDE { return new wxDockingEvent(*this); }
	virtual wxEventCategory GetEventCategory() const wxOVERRIDE { return wxEVT_CATEGORY_USER_INPUT; }

protected:
	void Assign(const wxDockingEvent &evt);

private:
	wxDockingInfo m_src;
	wxDockingInfo m_tgt;

	wxDECLARE_DYNAMIC_CLASS(wxDockingEvent);
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_EVENT_H_
