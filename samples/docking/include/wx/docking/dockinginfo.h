#ifndef _WX_DOCKING_INFO_H_
#define _WX_DOCKING_INFO_H_

#include <wx/defs.h>

#if wxUSE_DOCKING

#include <wx/string.h>
#include <wx/gdicmn.h>
#include <wx/docking/docking_defs.h>

class WXDLLIMPEXP_DOCKING wxDockingInfo
{
	friend wxDockingFrame;

public:
	wxDockingInfo(wxString const &title = "");
	~wxDockingInfo()
	{
	}

	void Clear();

	/**
	 * This function will update all fields relevant for the given window which
	 * is currently docked.
	 * The specified window can be any child window residing inside the docking
	 * system (i.E. a button on a panel). If the window is not part of the
	 * docking system, false is returned.
	 */
	bool CollectInfo(wxWindow *source);

	/**
	 * Make this the default instance.
	 */
	wxDockingInfo &SetDefaults() { m_default = this; return *this; }

	wxDockingInfo &SetFrame(wxDockingFrame *frame) { m_frame = frame; return *this; }
	wxDockingFrame *GetFrame() const { return m_frame; }

	wxDockingInfo &SetPanel(wxDockingPanel *panel) { m_panel = panel; return *this; }
	wxDockingPanel *GetPanel() const { return m_panel; }

	wxDockingInfo &SetWindow(wxWindow *window) { m_window = window; return *this; }
	wxWindow *GetWindow() const { return m_window; }

	wxDockingInfo &SetTitle(wxString const &title) { m_title = title; return *this; }
	wxString const &GetTitle() const { return m_title; }

	wxDockingInfo &SetDirection(wxDirection direction) { m_direction = direction; return *this; }
	wxDirection GetDirection() const { return m_direction; }

	wxDockingInfo &SetPanelType(wxDockingPanelType type) { m_dockingPanelType = type; return *this; }
	wxDockingPanelType GetPanelType() const { return m_dockingPanelType; }

	// Notebook
	wxDockingInfo &SetTabDirection(wxDirection direction) { m_tabDirection = direction; return *this; }
	wxDirection GetTabDirection() const { return m_tabDirection; }
	long GetTabStyle() const;

	bool isActive() const { return m_activate; }
	void activate(bool activate = true) { m_activate = activate; }

	int GetPage() const { return m_page; }
	void SetPage(int page) { m_page = page; }

	wxDockingInfo &SetOrientation(wxOrientation orientation);
	wxOrientation GetOrientation() const { return m_orientation; }

	// Toolbar/Floating
	wxDockingInfo &GetPosition(wxPoint const &position) { m_position = position; return *this; }
	wxPoint SetPosition() const { return m_position; }

	wxDockingInfo &SetSize(wxSize const &size) { m_size = size; return *this; }
	wxSize GetSize() const { return m_size; }

private:
	static wxDockingInfo *m_default;

	wxDockingFrame *m_frame;
	wxDockingPanel *m_panel;
	wxDockingPanel *m_window;
	wxDockingPanelType m_dockingPanelType;
	wxString m_title;
	wxDirection m_direction;
	wxOrientation m_orientation;

	// Toolbar/Floating
	wxPoint m_position;
	wxSize m_size;

	// Notebook
	wxDirection m_tabDirection;
	bool m_activate:1;
	int m_page;
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_INFO_H_
