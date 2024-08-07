/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/scrolbar.h
// Purpose:     wxScrollBar class
// Author:      Julian Smart
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SCROLBAR_H_
#define _WX_SCROLBAR_H_

#include "wx/platform.h"

#if wxUSE_GUI

// Scrollbar item
class WXDLLIMPEXP_CORE wxScrollBar: public wxScrollBarBase
{
public:
    wxScrollBar() { m_pageSize = 0; m_viewSize = 0; m_objectSize = 0; }
    virtual ~wxScrollBar();

    wxScrollBar(wxWindow *parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxSB_HORIZONTAL,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxScrollBarNameStr))
    {
        Create(parent, id, pos, size, style, validator, name);
    }
    bool Create(wxWindow *parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxSB_HORIZONTAL,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxScrollBarNameStr));

    int GetThumbPosition() const override;
    int GetThumbSize() const override { return m_pageSize; }
    int GetPageSize() const override { return m_viewSize; }
    int GetRange() const override { return m_objectSize; }

    virtual void SetThumbPosition(int viewStart) override;
    virtual void SetScrollbar(int position, int thumbSize, int range, int pageSize,
            bool refresh = true) override;

    // needed for RTTI
    void SetThumbSize( int s ) { SetScrollbar( GetThumbPosition() , s , GetRange() , GetPageSize() , true ) ; }
    void SetPageSize( int s ) { SetScrollbar( GetThumbPosition() , GetThumbSize() , GetRange() , s , true ) ; }
    void SetRange( int s ) { SetScrollbar( GetThumbPosition() , GetThumbSize() , s , GetPageSize() , true ) ; }

    void Command(wxCommandEvent& event) override;
    virtual bool MSWOnScroll(int orientation, WXWORD wParam,
                             WXWORD pos, WXHWND control) override;

    // override wxControl version to not use solid background here
    virtual WXHBRUSH MSWControlColor(WXHDC pDC, WXHWND hWnd) override;

    virtual WXDWORD MSWGetStyle(long style, WXDWORD *exstyle) const override;

    // returns true if the platform should explicitly apply a theme border
    virtual bool CanApplyThemeBorder() const override { return false; }

protected:
    virtual wxSize DoGetBestSize() const override;

    int m_pageSize;
    int m_viewSize;
    int m_objectSize;

    wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxScrollBar);
};

#endif

#endif
    // _WX_SCROLBAR_H_
