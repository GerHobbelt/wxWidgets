// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#if wxUSE_DOCKING

#include <wx/docking/dockingframe.h>
#include <wx/docking/dockingselector.h>
#include <wx/event.h>
#include <wx/dcclient.h>
#include <wx/colour.h>
#include <wx/brush.h>

wxBEGIN_EVENT_TABLE(wxDockingSelector, wxFrame)
	EVT_PAINT(wxDockingSelector::OnPaint)
wxEND_EVENT_TABLE()

wxDockingSelector::wxDockingSelector(wxDockingFrame *parent)
: wxFrame(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0L, wxASCII_STR(wxFrameNameStr))
, m_allowed(true)
{
}

wxDockingSelector::~wxDockingSelector()
{
}

void wxDockingSelector::Update(wxRect window, bool allowed)
{
	m_allowed = allowed;

	SetSize(window.width, window.height);
	Move(window.x, window.y);
}

void wxDockingSelector::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	// We dont use the wxPaintDC, because when redrawing the
	// window, Windows is in somce cases of the opinion that
	// we should redraw only a part of the window and since
	// we don't want to complicate matters, this leaves ugly
	// artifacts as the PaintDC is limited to those parts.
	// This is avoided by redrawing the full wxWindowDC
	// instead.
	wxPaintDC pdc(this);
	wxWindowDC dc(this);

	wxSize size = GetClientSize();
	if (m_allowed)
	{
		wxBrush green(wxColor(0u, 200u, 0u));
		dc.SetBrush(green);
	}
	else
		dc.SetBrush(*wxRED_BRUSH);

	dc.DrawRectangle(0, 0, size.x, size.y);
}

#endif // wxUSE_DOCKING
