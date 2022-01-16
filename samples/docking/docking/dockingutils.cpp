
#if wxUSE_DOCKING

#include <wx/app.h>
#include <wx/gbsizer.h>
#include <wx/dcscreen.h>

#include <wx/docking/dockingframe.h>

#ifndef wxUSE_NOTEBOOK
#error wxDockingFrame requires wxNotebook enabled
#endif

#ifndef wxUSE_SPLITTER
#error wxDockingFrame requires wxSplitterWindow enabled
#endif

using namespace std;

// TODO: We may need to look in all wxDockingFrames, instead of only one.
// If there are multiple floating windows around, this might not give the
// correct answer.
wxWindow *wxDockingWindowAtPoint(wxWindow *w, wxPoint const &pos)
{
	const wxWindowList &childs = w->GetChildren();
	for (wxWindowList::const_iterator it = childs.begin(); it != childs.end(); ++it)
	{
		w = *it;

		if (w->GetScreenRect().Contains(pos))
		{
			wxWindow *result = w;

			// Recursively find the innermost child.
			w = wxDockingWindowAtPoint(w, pos);
			if (w != NULL)
				result = w;

			return result;
		}
	}

	return NULL;
}

wxDockingFrame *wxDockingFrameFromWindow(wxWindow *w)
{
	while (w)
	{
		wxDockingFrame *frame = wxDynamicCast(w, wxDockingFrame);
		if (frame)
			return frame;

		w = w->GetParent();
	}

	return NULL;
}

WXDLLIMPEXP_DOCKING wxDockingPanel *wxDockingFindPanel(wxWindow *window, wxWindow **dockingChild, wxDockingPanelType *panelType)
{
	if (dockingChild)
		*dockingChild = NULL;

	wxWindow *child = NULL;
	while (window)
	{
		if (wxDockingFrame::CanDock(window, panelType))
		{
			if (dockingChild)
				*dockingChild = child;

			return window;
		}

		child = window;
		window = window->GetParent();
	}

	return NULL;
}

#endif // wxUSE_DOCKING
