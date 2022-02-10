/////////////////////////////////////////////////////////////////////////////
// Name:        wx/overlay.h
// Purpose:     wxOverlay class
// Author:      Stefan Csomor
// Modified by:
// Created:     2006-10-20
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OVERLAY_H_
#define _WX_OVERLAY_H_

#include "wx/defs.h"

#if wxUSE_GUI

#if defined(__WXDFB__)
    #define wxHAS_NATIVE_OVERLAY 1
#elif defined(__WXOSX__) && wxOSX_USE_COCOA
    #define wxHAS_NATIVE_OVERLAY 1
#elif defined(__WXMSW__)
    #include "wx/dcmemory.h"
    #include "wx/dcgraph.h"
    #define wxHAS_NATIVE_OVERLAY 1
    #define wxHAS_OVERLAYDC 1
#else
    // don't define wxHAS_NATIVE_OVERLAY
#endif

// ----------------------------------------------------------------------------
// creates an overlay over an existing window, allowing for manipulations like
// rubberbanding, etc. This API is not stable yet, not to be used outside wx
// internal code
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_FWD_CORE wxDC;
class WXDLLIMPEXP_FWD_CORE wxWindow;

class WXDLLIMPEXP_CORE wxOverlay
{
public:
    class Impl;

    wxOverlay();
    ~wxOverlay();

    // clears the overlay without restoring the former state
    // to be done eg when the window content has been changed and repainted
    void Reset();

    bool IsNative() const;

private:
    friend class WXDLLIMPEXP_FWD_CORE wxDCOverlay;

#ifdef wxHAS_OVERLAYDC
    friend class WXDLLIMPEXP_FWD_CORE wxOverlayDC;
    void Init(wxWindow* win, bool fullscreen);
#endif

    static Impl* Create();

    // returns true if it has been setup
    bool IsOk();

    void Init(wxDC* dc, int x , int y , int width , int height);

    void BeginDrawing(wxDC* dc);

    void EndDrawing(wxDC* dc);

    void Clear(wxDC* dc);

    Impl* m_impl;

    bool m_inDrawing;


    wxDECLARE_NO_COPY_CLASS(wxOverlay);
};


class WXDLLIMPEXP_CORE wxDCOverlay
{
public:
    // connects this overlay to the corresponding drawing dc, if the overlay is
    // not initialized yet this call will do so
    wxDCOverlay(wxOverlay &overlay, wxDC *dc, int x , int y , int width , int height);

    // convenience wrapper that behaves the same using the entire area of the dc
    wxDCOverlay(wxOverlay &overlay, wxDC *dc);

    // removes the connection between the overlay and the dc
    virtual ~wxDCOverlay();

    // clears the layer, restoring the state at the last init
    void Clear();

private:
    void Init(wxDC *dc, int x , int y , int width , int height);

    wxOverlay& m_overlay;

    wxDC* m_dc;


    wxDECLARE_NO_COPY_CLASS(wxDCOverlay);
};

//--------------------------------------------------------------
#ifdef wxHAS_OVERLAYDC

#if wxUSE_GRAPHICS_CONTEXT
    class wxOverlayDCBase : public wxGCDC
    {
    public:
        wxOverlayDCBase() : wxGCDC() {}
        ~wxOverlayDCBase() {}

    protected:
        void InitDC(wxBitmap& bitmap)
        {
            m_memDC.SelectObject(bitmap);
            SetGraphicsContext(wxGraphicsContext::Create(m_memDC));
        }

        wxMemoryDC m_memDC;
    };
#else // !wxUSE_GRAPHICS_CONTEXT
    class wxOverlayDCBase : public wxMemoryDC
    {
    public:
        wxOverlayDCBase() : wxMemoryDC() {}
        ~wxOverlayDCBase() {}

    protected:
        void InitDC(wxBitmap& bitmap)
        {
            SelectObject(bitmap);
        }
    };
#endif // wxUSE_GRAPHICS_CONTEXT

class WXDLLIMPEXP_CORE wxOverlayDC : public wxOverlayDCBase
{
public:
    wxOverlayDC(wxOverlay &overlay, wxWindow *win, int x, int y, int width, int height);
    wxOverlayDC(wxOverlay &overlay, wxWindow *win, bool fullscreen = false);

    virtual ~wxOverlayDC();

    // clears the layer
    void Clear();

    // sets the rectangle to be refreshed/updated within the overlay.
    void SetUpdateRectangle(const wxRect& rect);

private:
    void Init(wxWindow *win, bool fullscreen);

    wxOverlay& m_overlay;

    wxDECLARE_NO_COPY_CLASS(wxOverlayDC);
};

#endif // wxHAS_OVERLAYDC

#endif

#endif // _WX_OVERLAY_H_
