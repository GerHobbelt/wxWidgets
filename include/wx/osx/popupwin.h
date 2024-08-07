///////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/popupwin.h
// Purpose:     wxPopupWindow class for wxMac
// Author:      Stefan Csomor
// Created:
// Copyright:   (c) 2006 Stefan Csomor
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MAC_POPUPWIN_H_
#define _WX_MAC_POPUPWIN_H_

#include "wx/defs.h"

#if wxUSE_POPUPWIN

// ----------------------------------------------------------------------------
// wxPopupWindow
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPopupWindow : public wxPopupWindowBase
{
public:
    wxPopupWindow() { }
    ~wxPopupWindow();

    wxPopupWindow(wxWindow *parent, int flags = wxBORDER_NONE)
        { (void)Create(parent, flags); }

    bool Create(wxWindow *parent, int flags = wxBORDER_NONE);

    virtual bool Show(bool show = true) override;

    wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxPopupWindow);
};

#endif

#endif // _WX_MAC_POPUPWIN_H_

