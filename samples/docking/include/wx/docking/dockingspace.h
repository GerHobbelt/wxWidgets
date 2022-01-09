#pragma once

#include <wx/defs.h>

#if wxUSE_DOCKING

#include <wx/string.h>
#include <wx/docking/docking_defs.h>

class WXDLLIMPEXP_DOCKING wxDockingSpace
{
	friend wxDockingFrame;

public:
	wxDockingSpace(void)
	: m_frame(nullptr)
	, m_panel(nullptr)
	, m_window(nullptr)
	, m_direction(wxCENTRAL)
	, m_pageIndex(-1)
	, m_isNotebook(false)
	{
	}

	~wxDockingSpace()
	{
	}

	void clear()
	{
		m_frame = nullptr;
		m_panel = nullptr;
		m_window = nullptr;
		m_direction = wxCENTRAL;
		m_pageIndex = -1;
		m_isNotebook = false;
	}

	wxDockingFrame *getFrame(void) const { return m_frame; }
	void setFrame(wxDockingFrame *frame) { m_frame = frame; }

	wxDockingPanel *getPanel(void) const { return m_panel; }
	void setPanel(wxDockingPanel *panel) { m_panel = panel; }

	wxWindow *getWindow(void) const { return m_window; }
	void setWindow(wxWindow *window) { m_window = window; }

	wxDirection getDirection(void) const { return m_direction; }
	void setDirection(wxDirection direction) { m_direction = direction; }

	int getPageIndex(void) const { return m_pageIndex; }
	void setPageIndex(int pageIndex) { m_pageIndex = pageIndex; }

	bool isNotebook(void) const { return m_isNotebook; }
	void setNotebook(bool notebook) { m_isNotebook = notebook; }

private:
	wxDockingFrame *m_frame;
	wxDockingPanel *m_panel;
	wxDockingPanel *m_window;
	wxDirection m_direction;
	int m_pageIndex;
	bool m_isNotebook;
};

#endif // wxUSE_DOCKING
