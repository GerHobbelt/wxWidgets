#pragma once

#include "wx/defs.h"

#if wxUSE_DOCKING
#include <vector>

#include <wx/frame.h>

#include <wx/docking/docking_defs.h>
#include <wx/docking/dockinginfo.h>

class wxGridBagSizer;
class wxNotebook;

/**
 * wxDockingFrame provides the main frame window handling docking
 * to it's layout.
 * Direction must always be one of wxNONE(center), wxLEFT, wxRIGHT, wxUP, wxDOWN.
 */
class WXDLLIMPEXP_DOCKING wxDockingFrame
: public wxFrame
{
public:
	wxDockingFrame();

	wxDockingFrame(wxWindow *parent,
		wxWindowID id,
		const wxString &title,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString &name = wxASCII_STR(wxFrameNameStr));

	~wxDockingFrame() override;

	bool Create(wxWindow *parent,
		wxWindowID id,
		const wxString &title,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString &name = wxASCII_STR(wxFrameNameStr));

	wxDockingInfo &Defaults(void) { return m_defaults; }
	wxDockingInfo const &Defaults(void) const { return m_defaults; }

	/**
	 * Add the panel to a tab in the dockiongPanel. If the dockingPanel doesn't
	 * have tabs, it will be converted accordingly.
	 * If the notebook pointer is provided, the notebook will be returned as
	 * well. If showTab() is not set in the info and it is the first panel,
	 * then no tab is created and this pointer will be null.
	 */
	wxDockingPanel *AddTabPanel(wxWindow *panel, wxDockingInfo const &info);

	/**
	 * Split the dockingPanel and add the panel in the specified direction.
	 */
	wxDockingPanel *SplitPanel(wxWindow *panel, wxDockingInfo const &info);

	/**
	 * Create a floating dockingPanel and insert the panel.
	 */
	wxDockingPanel *FloatPanel(wxWindow *panel, wxDockingInfo const &info);

	/**
	 * Adds a new toolbar to the frame.
	 */
	wxDockingPanel *AddToolBar(wxToolBar *toolbar, wxDockingInfo const &info);

	/**
	 * HideToolbar will hide the toolbar, so it is no longer visible. The toolbar
	 * is still associated to the frame, and can be shown again. It may not be
	 * reattached by AddToolBar().
	 */
	wxDockingPanel *HideToolBar(wxToolBar *toolbar, wxDockingInfo const &info);

	/**
	 * RemoveToolbar will detach the toolbar from the frame. After this, the toolbar
	 * is no longer associated with a window, and the user is responsible for deleting
	 * the toolbar. After this the toolbar has to be reattached by using AddToolBar()
	 * if desired.
	 */
	bool RemoveToolBar(wxToolBar *toolbar, wxDockingInfo const &info);

	/**
	 * Remove the specified window from the docking. If the panel is part of a notebook
	 * the page will be removed. If it was the last page, the notebook will also be deleted.
	 * If the serwindow is part of a splitter, or a notebook inside a splitter, the split
	 * window is unsplitted and also removed. Because of this, this can cause a chainreaction
	 * of windows being removed.
	 *
	 * Note: The client should never hold on to a dockingpanel, splitter or notebook as those
	 * might be deleted in the process and are handled internally.
	 *
	 * RETURN: The panel that it was docked to and still exists. So if panels had been recursively
	 * removed, it will be the last panel that has not been removed.
	 */
	wxDockingPanel *RemovePanel(wxWindow *userWindow);

	/**
	 * Serialize the current layout to a string, which allows to restore this layout later.
	 * This string is also suitable for persisting the layout into a config file to be able
	 * to restore the layout after the application restarts.
	 */
	//wxString SerializeLayout(void) const;

	/**
	 * Only serialize this frame.
	 */
	//wxString SerializeFrame(void) const;

	/**
	 * Deserialize the layout from a string created by serializeLayout(). It should be noted
	 * that the application is responsible for creating the appropriate panels, as this function
	 * can only restore the layout itself, but not the panels created by the application.
	 */
	//bool DeserializeLayout(wxString layout);

	/**
	 * Only deserialize this frame.
	 */
	//bool DeserializeFrame(wxString layout);

	/**
	 * Find the parent we can dock to. If the provided window is already dockable, this is
	 * returned.
	 */
	wxDockingPanel *FindDockingPanel(wxWindow *window) const;

	/**
	 * Returns true if the window can be docked to. By default a window can be docked to
	 * if it is either a wxNotebook, a wxSplitterWindow or a direct child of one of them.
	 */
	virtual bool isDockable(wxWindow *window) const;

public:
	void OnSize(wxSizeEvent &event);

protected:
	void DoSize(void);

	/**
	 * Create a wxNotebook tab panel with the userWindow as it's page. If the userWindow is
	 * a wxNotebook it will not create a new one, instead it adds the userWindow to it. If the
	 * parent is a nullpointer the parent of the user window will be used as the parent for the
	 * wxDockingTarget. If the info doesn't contain a docking target, a new wxDockingPanel will
	 * be created.
	 */
	wxDockingPanel *CreateTabPanel(wxWindow *userWindow, wxDockingInfo const &info, wxWindow*parent = nullptr);

	/**
	 * Remove the panel from the docking. The panel is not destroyed itself, even though the docked
	 * panel can be destroyed if it becomes empty. The panel can still be docked to some other
	 * target.
	 */
	//bool RemovePanel(wxDockingPanel *panel);

	void OnMouseLeftDown(wxMouseEvent &event);

	virtual void SetActivePanel(wxDockingPanel *panel);
	wxDockingPanel *GetActivePanel(void) const
	{
		return m_activePanel;
	}

	void UpdateToolbarLayout(void);
	bool HideToolbar(wxDockingPanel *&toolbar);

	/**
	 * Replace a page from a notebook with a new notebook containing the original page.
	 * Returns the page and the index if successfull, or nullptr if the page was not found.
	 */
	wxNotebook *ReplaceNotebookPage(wxNotebook *notebook, wxWindow *oldPage, int &index, wxDockingInfo const &info);

private:
	void init(void);
	void BindEventHandlers(void);
	void UnbindEventHandlers(void);

private:
	wxWindow *m_rootPanel;

	// Some values are used as defaults if not specified in a function.
	wxDockingInfo m_defaults;

	// Toolbar members
	wxGridBagSizer *m_sizer;
	wxDockingPanel *m_activePanel;
	wxDockingPanel *m_toolbarsLeft;
	wxDockingPanel *m_toolbarsRight;
	wxDockingPanel *m_toolbarsTop;
	wxDockingPanel *m_toolbarsBottom;

	wxDECLARE_DYNAMIC_CLASS(wxDockingFrame);
};

#endif // wxUSE_DOCKING
