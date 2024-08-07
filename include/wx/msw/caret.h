///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/caret.h
// Purpose:     wxCaret class - the MSW implementation of wxCaret
// Author:      Vadim Zeitlin
// Created:     23.05.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CARET_H_
#define _WX_CARET_H_

#include "wx/platform.h"

#if wxUSE_GUI

class WXDLLIMPEXP_CORE wxCaret : public wxCaretBase
{
public:
    wxCaret() { Init(); }
        // create the caret of given (in pixels) width and height and associate
        // with the given window
    wxCaret(wxWindow *window, int width, int height)
    {
        Init();

        (void)Create(window, width, height);
    }
        // same as above
    wxCaret(wxWindowBase *window, const wxSize& size)
    {
        Init();

        (void)Create(window, size);
    }

    // process wxWindow notifications
    virtual void OnSetFocus() override;
    virtual void OnKillFocus() override;

protected:
    // override base class virtuals
    virtual void DoMove() override;
    virtual void DoShow() override;
    virtual void DoHide() override;
    virtual void DoSize() override;

    // helper function which creates the system caret
    bool MSWCreateCaret();

private:
    void Init()
    {
        m_hasCaret = false;
    }

    bool m_hasCaret;

    wxDECLARE_NO_COPY_CLASS(wxCaret);
};

#endif

#endif // _WX_CARET_H_


