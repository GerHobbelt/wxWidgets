#ifndef _WX_DOCKING_FRAME_H_
#define _WX_DOCKING_FRAME_H_

#include "wx/defs.h"

#if wxUSE_DOCKING

#include <wx/gbsizer.h>
#include <wx/frame.h>
#include <wx/splitter.h>
#include <wx/notebook.h>

#include <wx/docking/dockingevent.h>
#include <wx/docking/docking_defs.h>
#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingutils.h>

class wxDockingSelector;

/**
 * wxDockingFrame provides the main frame window handling docking
 * to it's layout.
 * Direction must always be one of wxNONE(center), wxLEFT, wxRIGHT, wxUP, wxDOWN.
 */
class WXDLLIMPEXP_DOCKING wxDockingFrame
: public wxFrame
{
	friend class wxDockingEventFilter;
	friend wxDockingPanel *wxDockingFindPanel(wxWindow *window, wxWindow **dockingChild, wxDockingPanelType *panelType);

public:
	wxDockingFrame();

	wxDockingFrame(wxWindow *parent,
		wxWindowID id,
		const wxString &title,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString &name = wxASCII_STR(wxFrameNameStr));

	~wxDockingFrame() wxOVERRIDE;

	bool Create(wxWindow *parent,
		wxWindowID id,
		const wxString &title,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString &name = wxASCII_STR(wxFrameNameStr));

	wxDockingInfo &Defaults() { return m_defaults; }
	wxDockingInfo const &Defaults() const { return m_defaults; }

	wxDockingPanel *AddPanel(wxWindow *window, wxDockingInfo const &info);

	/**
	 * Moves a window, which already has to exist in the docking system, to
	 * a new destination. If the window does not exist in the docking system
	 * false is returned.
	 */
	bool MovePanel(wxWindow *source, wxDockingInfo &target);

	/**
	 * Moves a window, which already has to exist in the docking system, to
	 * a new destination. If the window does not exist in the docking system
	 * false is returned.
	 */
	bool MovePanel(wxDockingInfo &source, wxDockingInfo &target);

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
	 * The window provided by the suer, will always be reparented to the wxDockingFrame. So if it
	 * is to be resued, it might need to be reparented appropriatly. If it is to be docked again
	 * somewhere else, then this is not needed, as the docking module will do this internally to
	 * the correct target panel.
	 *
	 * Note: The client should never hold on to a dockingpanel, splitter or notebook as those
	 * might be deleted in the process and are handled internally.
	 *
	 * RETURN: The panel that replaced the docking panel the window was connected to. In case
	 * of a notebook, if the notebook still exists, this will be the notebook itself. In case
	 * of a splitter, this is usually the parent.
	 */
	wxDockingPanel *RemovePanel(wxDockingInfo &info);
	wxDockingPanel *RemovePanel(wxWindow *userWindow);

	/**
	 * Serialize the current layout to a string, which allows to restore this layout later.
	 * This string is also suitable for persisting the layout into a config file to be able
	 * to restore the layout after the application restarts.
	 */
	//wxString SerializeLayout() const;

	/**
	 * Only serialize this frame.
	 */
	//wxString SerializeFrame() const;

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
	 * Returns true if the window can be docked to. By default a window can be docked to
	 * if it is either a wxNotebook, a wxSplitterWindow or a direct child of one of them.
	 * If the window is dockable and is a known type, then this will be reported if the
	 * pointer is not a nullptr.
	 */
	static bool isDockable(wxWindow *window, wxDockingPanelType *panelType = nullptr);

public:
	void OnSize(wxSizeEvent &event);

protected:
	void DoSize();

	/**
	 * Move panel only when source and target are both in the same splitter.
	 */
	bool DoMoveSplitter(wxDockingInfo &src, wxDockingInfo &tgt);

	wxDockingPanel *RemoveFromNotebook(wxWindow *page, wxNotebook *notebook);
	wxDockingPanel *RemoveFromSplitter(wxWindow *parent, wxWindow *window, wxSplitterWindow *splitter);

	wxDockingPanel *GetRootPanel() { return m_rootPanel; }

	wxNotebook *CreateNotebook(wxWindow *parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0);
	void DeleteNotebook(wxNotebook *notebook);

	wxSplitterWindow *CreateSplitter(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSP_3D);
	void DeleteSplitter(wxSplitterWindow *splitter);

	/**
	 * Create a wxNotebook tab panel with the userWindow as it's page. If the userWindow is
	 * a wxNotebook it will not create a new one, instead it adds the userWindow to it. If the
	 * parent is a nullpointer the parent of the user window will be used as the parent for the
	 * wxDockingTarget. If the info doesn't contain a docking target, a new wxDockingPanel will
	 * be created.
	 */
	wxDockingPanel *CreateTabPanel(wxWindow *userWindow, wxDockingInfo const &info, wxWindow*parent = nullptr);

	int OnMouseLeftDown(wxMouseEvent &event);
	int OnMouseLeftUp(wxMouseEvent &event);
	int OnMouseMove(wxMouseEvent &event);

	void OnSplitterSashUpdate(wxSplitterEvent &event);
	void OnSplitterDClick(wxSplitterEvent &event);

	virtual void SetActivePanel(wxDockingPanel *panel);
	wxDockingPanel *GetActivePanel() const
	{
		return m_activePanel;
	}

	void UpdateToolbarLayout();
	bool HideToolbar(wxDockingPanel *&toolbar);

	/**
	 * Replace a page from a notebook with a new notebook containing the original page.
	 * Returns the page and the index if successfull, or nullptr if the page was not found.
	 */
	wxNotebook *ReplaceNotebookPage(wxNotebook *notebook, wxWindow *oldPage, int &index, wxDockingInfo const &info);

	/**
	 * Set the size of the bar that indicates the docking target.
	 * This is also threshold where the mouse is allowed to start a docking dragging operation.
	 * i.E. If set to 20 the mouse can be up to 20 pixels away from the top border to start dragging.
	 */
	void setDockingWidth(uint32_t width) { m_dockingWidth = width; }
	uint32_t getDockingWidth() const { return m_dockingWidth; }

	/**
	 * Show the overlay which indicates the docking target for the user. By default, this will draw
	 * source window with a transparency effect.
	 *
	 * @param position Position of the overlay
	 * @param size Size of the overlay
	 * @param allowed Indicates if the docking operation is allowed at the currently selected
	 *					target location
	 */
	void ShowSelectorOverlay(wxRect const &window, bool allowed);
	void HideSelectorOverlay(bool del = false);

	/**
	 * Calculate the size of the overlay window if the mouse position is inside an area which is suitable for docking.
	 * If the area is suitable for docking, then true is returned, and the direction and size for the overlay is returned.
	 * Note that the return value does not indicate if the docking is actually allowed. It only tells if it is potentially
	 * possible to dock at the specified mouse position.
	 *
	 * @param mousePos The mouse position to check for (always in screen coordinates)
	 * @param curWindow The window the mouse cursor is currently hovering over.
	 * @param [OUT] The direction the docking would happen to
	 * @param [OUT] The size of the rectangle the overlay should occupy (always in screen coordinates)
	 */
	bool CalcOverlayRectangle(wxPoint const &mousePos, wxWindow *curWindow, wxDirection &direction, wxRect &windowRectangle);

	bool InitSourceEvent(wxPoint const &mousePos);
	bool CheckNotebook(wxPoint const &mousePos, wxDockingInfo &info);

	/**
	 * A helper function which is needed to avoid recursion.
	 */
	static bool CanDock(wxWindow *window, wxDockingPanelType *panelType);

private:
	void init();
	void BindEventHandlers();
	void UnbindEventHandlers();

private:
	wxDockingPanel *m_rootPanel;

	// Some values are used as defaults if not specified in a function.
	wxDockingInfo m_defaults;

	// Toolbar members
	wxGridBagSizer *m_sizer;
	wxDockingPanel *m_activePanel;
	wxDockingPanel *m_toolbarsLeft;
	wxDockingPanel *m_toolbarsRight;
	wxDockingPanel *m_toolbarsTop;
	wxDockingPanel *m_toolbarsBottom;
	wxDockingSelector *m_selector;

	wxDockingEvent m_event;
	wxDockingInfo m_lastTarget;

	uint32_t m_dockingWidth;

	bool m_mouseCaptured : 1;

	wxDECLARE_DYNAMIC_CLASS(wxDockingFrame);
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_FRAME_H_
