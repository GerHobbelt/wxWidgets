/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private/overlay.h
// Purpose:     wxOverlayImpl declaration
// Author:      Kettab Ali
// Created:     2021-07-12
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_PRIVATE_OVERLAY_H_
#define _WX_MSW_PRIVATE_OVERLAY_H_

#include "wx/bitmap.h"

class WXDLLIMPEXP_FWD_CORE wxWindow;
class WXDLLIMPEXP_FWD_CORE wxDC;

class wxOverlayImpl
{
public:
    wxOverlayImpl();
    ~wxOverlayImpl();

    void Reset();
    bool IsOk();
    void Init(wxDC* dc, int x , int y , int width , int height);
    void Init(wxWindow* win, bool fullscreen);
    void BeginDrawing(wxDC* dc);
    void EndDrawing(wxDC* dc);
    void Clear(wxDC* dc);

    void SetUpdateRectangle(const wxRect& rect);

    wxBitmap& GetBitmap() { return m_buffer; }

public:
    // window the overlay is associated with
    wxWindow* m_window;
    // the overlay window itself
    wxWindow* m_overlayWindow;
    // rectangle to be refreshed/updated within the overlay,
    // in m_window's window coordinates if not fullScreen.
    wxRect m_rect;
    wxRect m_oldRect;

    bool m_fullscreen;

    // wxOverlayDC draws on this off-screen bitmap
    wxBitmap m_buffer;
};

#endif // _WX_MSW_PRIVATE_OVERLAY_H_
