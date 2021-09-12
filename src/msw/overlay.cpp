/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/overlay.cpp
// Purpose:     wxOverlay implementation for wxMSW
// Author:      Kettab Ali
// Created:     2021-07-12
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/window.h"
    #include "wx/dcclient.h"
#endif

#include "wx/display.h"
#include "wx/nativewin.h"
#include "wx/scrolwin.h"
#include "wx/private/overlay.h"
#include "wx/msw/private.h"

#ifdef wxHAS_NATIVE_OVERLAY

// Quoting the MSDN:
//
//   "UpdateLayeredWindow always updates the entire window. To update part of a window,
//    use the traditional WM_PAINT and set the blend value using SetLayeredWindowAttributes.
//
//    For best drawing performance by the layered window and any underlying windows,
//    the layered window should be as small as possible."
//
// Therfore setting the transparency of the layered window with SetLayeredWindowAttributes
// is preferred over UpdateLayeredWindow.

//#define wxUSE_UPDATELAYEREDWINDOW

namespace wxPrivate
{
static const wxChar* gs_overlayClassName = NULL;

class wxOverlayWindow : public wxNativeContainerWindow
{
public:
    static wxWindow*  Create()
    {
        static const wxChar* OVERLAY_WINDOW_CLASS = wxS("wxOverlayWindow");

        if ( !gs_overlayClassName )
        {
            WNDCLASS wndclass;
            wxZeroMemory(wndclass);

            wndclass.lpfnWndProc   = ::DefWindowProc;
            wndclass.hInstance     = wxGetInstance();
            wndclass.lpszClassName = OVERLAY_WINDOW_CLASS;

            if ( !::RegisterClass(&wndclass) )
            {
                wxLogLastError(wxS("RegisterClass() in wxOverlayWindow::Create()"));
                return NULL;
            }

            gs_overlayClassName = OVERLAY_WINDOW_CLASS;
        }

        HWND hwnd = ::CreateWindowEx
                      (
                        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOPMOST,
                        OVERLAY_WINDOW_CLASS,
                        NULL,
                        WS_POPUP, 0, 0, 0,
                        0,
                        (HWND) NULL,
                        (HMENU)NULL,
                        wxGetInstance(),
                        (LPVOID) NULL
                      );

        if ( !hwnd )
        {
            wxLogLastError(wxS("CreateWindowEx() in wxOverlayWindow::Create()"));
            return NULL;
        }

#ifndef wxUSE_UPDATELAYEREDWINDOW
        if ( !::SetLayeredWindowAttributes(hwnd, 0, 128, LWA_COLORKEY|LWA_ALPHA) )
            wxLogLastError(wxS("SetLayeredWindowAttributes() in wxOverlayWindow::Create()"));
#endif // wxUSE_UPDATELAYEREDWINDOW

        return new wxOverlayWindow(hwnd);
    }

    virtual bool Destroy() wxOVERRIDE
    {
        HWND hwnd = GetHandle();

        if ( hwnd && !::DestroyWindow(hwnd) )
        {
            wxLogLastError(wxS("DestroyWindow in wxOverlayWindow::Destroy()"));
            return false;
        }

        if ( gs_overlayClassName )
        {
            if ( !::UnregisterClass(gs_overlayClassName, wxGetInstance()) )
            {
                wxLogLastError(wxS("UnregisterClass in wxOverlayWindow::Destroy()"));
            }

            gs_overlayClassName = NULL;
        }

        return true;
    }

    virtual bool Show(bool show) wxOVERRIDE
    {
        if ( !wxWindowBase::Show(show) || !GetHandle() )
            return false;

        if ( show )
        {
            ::SetWindowPos(GetHandle(), NULL, 0, 0, 0, 0,
                           SWP_NOSIZE |
                           SWP_NOMOVE |
                           SWP_NOREDRAW |
                           SWP_NOOWNERZORDER |
                           SWP_NOACTIVATE |
                           SWP_SHOWWINDOW);
        }
        else
        {
            ::ShowWindow(GetHandle(), SW_HIDE);
        }

        return true;
    }

private:
    wxOverlayWindow(HWND hwnd) : wxNativeContainerWindow(hwnd) {}
    ~wxOverlayWindow() {}
};
} // wxPrivate namespace

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxOverlay
// ----------------------------------------------------------------------------

wxOverlayImpl::wxOverlayImpl()
{
    m_window = NULL;
    m_overlayWindow = NULL;
}

wxOverlayImpl::~wxOverlayImpl()
{
    Reset();

    m_overlayWindow->Destroy();
}

bool wxOverlayImpl::IsOk()
{
    return m_buffer.IsOk();
}

void wxOverlayImpl::Init(wxWindow* win, bool fullscreen)
{
    wxASSERT_MSG( !IsOk() , "You cannot Init an overlay twice" );

    m_window = win;
    m_fullscreen = fullscreen;

    if ( fullscreen )
    {
        m_rect = wxDisplay(win).GetGeometry();
    }
    else
    {
        m_rect.SetSize(win->GetClientSize());
        m_rect.SetPosition(win->GetScreenPosition());
    }

    m_buffer.Create(m_rect.GetWidth(), m_rect.GetHeight());

    if ( !m_overlayWindow )
    {
        m_overlayWindow = wxPrivate::wxOverlayWindow::Create();
    }

    m_overlayWindow->Move(m_rect.GetPosition());
    m_overlayWindow->SetSize(m_rect.GetSize());
    m_overlayWindow->Show();
}

void wxOverlayImpl::Init(wxDC* , int, int, int, int)
{
    wxFAIL_MSG("wxOverlay initialized from wxDC not implemented under wxMSW");
}

void wxOverlayImpl::SetUpdateRectangle(const wxRect& rect)
{
    m_rect = rect;
}

void wxOverlayImpl::BeginDrawing(wxDC* dc)
{
    wxScrolledWindow* const win = wxDynamicCast(m_window, wxScrolledWindow);
    if ( win )
        win->PrepareDC(*dc);
}

void wxOverlayImpl::EndDrawing(wxDC* dc)
{
#ifdef wxUSE_UPDATELAYEREDWINDOW
    HWND hWnd = (HWND)m_overlayWindow->GetHandle();

    BLENDFUNCTION blendFnc = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

    const wxPoint pt = m_overlayWindow->GetPosition();
    const wxSize sz  = m_overlayWindow->GetSize();

    SIZE szDst  = {sz.GetWidth(), sz.GetHeight()};
    POINT ptDst = {pt.x, pt.y}; // In screen coordinates
    POINT ptSrc = {0, 0};

    if ( !::UpdateLayeredWindow(hWnd, ScreenHDC(), &ptDst, &szDst,
                                (HDC)dc->GetHandle(), &ptSrc, 0, &blendFnc, ULW_ALPHA) )
    {
        wxLogLastError(wxS("UpdateLayeredWindow() failed!"));
    }
#else // wxUSE_UPDATELAYEREDWINDOW
    int x = wxMin(m_rect.x, m_oldRect.x),
        y = wxMin(m_rect.y, m_oldRect.y);

    int w = wxMax(m_rect.width, m_oldRect.width),
        h = wxMax(m_rect.height, m_oldRect.height);

    m_oldRect = m_rect;

    m_rect = wxRect(x, y, w, h);

    wxWindowDC winDC(m_overlayWindow);
    const wxPoint pt = dc->GetDeviceOrigin();

    winDC.Blit(m_rect.x, m_rect.y, m_rect.width, m_rect.height, dc, -pt.x+m_rect.x, -pt.y+m_rect.y);
#endif // !wxUSE_UPDATELAYEREDWINDOW
}

void wxOverlayImpl::Clear(wxDC* dc)
{
    wxASSERT_MSG( IsOk(),
                  "You cannot Clear an overlay that is not initialized" );

    dc->SetBackground(wxBrush(wxTransparentColour));
    dc->Clear();
}

void wxOverlayImpl::Reset()
{
    m_buffer = wxBitmap();

    if ( m_overlayWindow )
        m_overlayWindow->Hide();
}

#endif // wxHAS_NATIVE_OVERLAY
