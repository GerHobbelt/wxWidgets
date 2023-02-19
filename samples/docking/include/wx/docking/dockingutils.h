#ifndef _WX_DOCKING_UTILS_H_
#define _WX_DOCKING_UTILS_H_

#include "wx/defs.h"

#if wxUSE_DOCKING

#include <wx/toolbar.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/docking/docking_defs.h>
#include <wx/docking/dockinginfo.h>

class WXDLLIMPEXP_DOCKING wxDockingPanelPtr
{
public:
	wxDockingPanelPtr()
	: m_window(nullptr)
	, m_type(wxDOCKING_NONE)
	{
	}

	wxDockingPanelPtr(wxWindow *window)
	{
		Set(window);
	}

	static wxDockingPanelType CheckType(wxWindow *window) { wxDockingPanelPtr w(window); return w.GetType(); }
	wxDockingPanelType Set(wxWindow *window);

	wxWindow *GetWindow() const { return m_window; }
	void SetWindow(wxWindow *window) { m_window = window; m_type = wxDOCKING_WINDOW; }

	wxNotebook *GetNotebook() const { return (m_type == wxDOCKING_NOTEBOOK) ? m_notebook : nullptr; }
	void SetNotebook(wxNotebook *window) { m_notebook = window; m_type = wxDOCKING_NOTEBOOK; }

	wxSplitterWindow *GetSplitter() const { return (m_type == wxDOCKING_SPLITTER) ? m_splitter : nullptr; }
	void SetSplitter(wxSplitterWindow *window) { m_splitter = window; m_type = wxDOCKING_SPLITTER; }

	wxToolBar *GetToolBar() const { return (m_type == wxDOCKING_TOOLBAR) ? m_toolbar : nullptr; }
	void SetToolBar(wxToolBar *window) { m_toolbar = window; m_type = wxDOCKING_TOOLBAR; }

	wxDockingFrame *GetFrame() const { return (m_type == wxDOCKING_FRAME) ? m_frame : nullptr; }
	void SetFrame(wxDockingFrame *window) { m_frame = window; m_type = wxDOCKING_FRAME; }

	wxDockingPanelType GetType() const { return m_type; }

private:
	union
	{
		wxWindow			*m_window;
		wxSplitterWindow	*m_splitter;
		wxNotebook			*m_notebook;
		wxToolBar			*m_toolbar;
		wxDockingFrame		*m_frame;
	};

	wxDockingPanelType m_type;
};

/**
 * The original wxFindWindowAtPoint doesn't allow us to ignore a window, so here we are using our own
 * in order to ignore the overlay window, we use to indicate the docking target. This function
 * takes a window as parameter and only childs of that window are taken into consideration.
 *
 * @param window is the 
 */
WXDLLIMPEXP_DOCKING wxWindow *wxDockingWindowAtPoint(wxWindow *window, wxPoint const &screenPos);

/**
 * Find the wxDockingFrame from the specified window.
 */
WXDLLIMPEXP_DOCKING wxDockingFrame *wxDockingFrameFromWindow(wxWindow *window);

/**
 * Find the parent we can dock to. If the provided window is already dockable, this is
 * returned.
 * dockingChild returns the child window which is directly connected to the dockingpanel.
 * If window is the dockingPanel itself, then dockingChild is nullptr.
 */
WXDLLIMPEXP_DOCKING wxDockingPanel *wxDockingFindPanel(wxWindow *window, wxWindow **dockingChild = nullptr, wxDockingPanelType *panelType = nullptr);

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_UTILS_H_
