#ifndef _WX_DOCKING_SELECTOR_H_
#define _WX_DOCKING_SELECTOR_H_

#include <wx/defs.h>

#if wxUSE_DOCKING

#include <wx/docking/docking_defs.h>
#include <wx/frame.h>

class WXDLLIMPEXP_DOCKING wxDockingSelector
: public wxFrame
{
public:
	wxDockingSelector(wxDockingFrame *parent);
	~wxDockingSelector();

	void Update(wxRect window, bool allowed);

protected:
	void OnPaint(wxPaintEvent &WXUNUSED(evt));

private:
	bool m_allowed;

	wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_SELECTOR_H_
