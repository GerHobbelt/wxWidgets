/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/glcanvas.h
// Purpose:     wxGLCanvas, for using OpenGL with wxWidgets under Windows
// Author:      Julian Smart
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GLCANVAS_H_
#define _WX_GLCANVAS_H_

#include "wx/platform.h"

#if wxUSE_GUI

#include "wx/palette.h"

#include "wx/msw/wrapwin.h"

#include <GL/gl.h>

// ----------------------------------------------------------------------------
// wxGLContext: OpenGL rendering context
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_GL wxGLContext : public wxGLContextBase
{
public:
    wxGLContext(wxGLCanvas *win,
                const wxGLContext *other = nullptr,
                const wxGLContextAttrs *ctxAttrs = nullptr);
    virtual ~wxGLContext();

    virtual bool SetCurrent(const wxGLCanvas& win) const override;

    HGLRC GetGLRC() const { return m_glContext; }

protected:
    HGLRC m_glContext;

private:
    wxDECLARE_CLASS(wxGLContext);
};

// ----------------------------------------------------------------------------
// wxGLCanvas: OpenGL output window
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_GL wxGLCanvas : public wxGLCanvasBase
{
public:
    wxGLCanvas() = default;

    explicit // avoid implicitly converting a wxWindow* to wxGLCanvas
    wxGLCanvas(wxWindow *parent,
               const wxGLAttributes& dispAttrs,
               wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxString& name = wxGLCanvasName,
               const wxPalette& palette = wxNullPalette);

    explicit
    wxGLCanvas(wxWindow *parent,
               wxWindowID id = wxID_ANY,
               const int *attribList = nullptr,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxString& name = wxGLCanvasName,
               const wxPalette& palette = wxNullPalette);

    bool Create(wxWindow *parent,
                const wxGLAttributes& dispAttrs,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxGLCanvasName,
                const wxPalette& palette = wxNullPalette);

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxGLCanvasName,
                const int *attribList = nullptr,
                const wxPalette& palette = wxNullPalette);

    virtual ~wxGLCanvas();

    // implement wxGLCanvasBase methods
    virtual bool SwapBuffers() override;


    // MSW-specific helpers
    // --------------------

    // get the HDC used for OpenGL rendering
    HDC GetHDC() const { return m_hDC; }

    // Try to find pixel format matching the given attributes list for the
    // specified HDC, return 0 on error, otherwise ppfd is filled in with the
    // information from dispAttrs
    static int FindMatchingPixelFormat(const wxGLAttributes& dispAttrs,
                                       PIXELFORMATDESCRIPTOR* ppfd = nullptr);
    // Same as FindMatchingPixelFormat
    static int ChooseMatchingPixelFormat(HDC hdc, const int *attribList,
                                         PIXELFORMATDESCRIPTOR *pfd = nullptr);

#if wxUSE_PALETTE
    // palette stuff
    bool SetupPalette(const wxPalette& palette);
    virtual wxPalette CreateDefaultPalette() override;
    void OnQueryNewPalette(wxQueryNewPaletteEvent& event);
    void OnPaletteChanged(wxPaletteChangedEvent& event);
#endif // wxUSE_PALETTE

protected:
    // the real window creation function, Create() may reuse it twice as we may
    // need to create an OpenGL window to query the available extensions and
    // then potentially delete and recreate it with another pixel format
    bool CreateWindow(wxWindow *parent,
                      wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      long style = 0,
                      const wxString& name = wxGLCanvasName);

    // set up the pixel format using the given attributes and palette
    int DoSetup(PIXELFORMATDESCRIPTOR &pfd, const int *attribList);


    // HDC for this window, we keep it all the time
    HDC m_hDC = nullptr;

private:
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_CLASS(wxGLCanvas);
};

#endif

#endif // _WX_GLCANVAS_H_
