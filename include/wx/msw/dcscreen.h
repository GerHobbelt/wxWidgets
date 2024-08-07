/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/dcscreen.h
// Purpose:     wxScreenDC class
// Author:      Julian Smart
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_DCSCREEN_H_
#define _WX_MSW_DCSCREEN_H_

#include "wx/dcscreen.h"
#include "wx/msw/dc.h"

#if wxUSE_GUI

class WXDLLIMPEXP_CORE wxScreenDCImpl : public wxMSWDCImpl
{
public:
    // Create a DC representing the whole virtual screen (all monitors)
    wxScreenDCImpl( wxScreenDC *owner );

    // Return the size of the whole virtual screen (all monitors)
    virtual void DoGetSize(int *w, int *h) const override;

    wxDECLARE_CLASS(wxScreenDCImpl);
    wxDECLARE_NO_COPY_CLASS(wxScreenDCImpl);
};

#endif

#endif // _WX_MSW_DCSCREEN_H_

