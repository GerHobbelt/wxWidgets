#ifndef _WX_DOCKING_UTILS_H_
#define _WX_DOCKING_UTILS_H_

#include "wx/defs.h"

#if wxUSE_DOCKING

#include <wx/docking/docking_defs.h>
#include <wx/docking/dockinginfo.h>

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
 * If window is the dockingPanel itself, then dockingChild is NULL.
 */
WXDLLIMPEXP_DOCKING wxDockingPanel *wxDockingFindPanel(wxWindow *window, wxWindow **dockingChild = NULL, wxDockingPanelType *panelType = NULL);

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_UTILS_H_
