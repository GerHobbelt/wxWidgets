/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dcmemory.h
// Purpose:     wxMemoryDC base header
// Author:      Julian Smart
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCMEMORY_H_BASE_
#define _WX_DCMEMORY_H_BASE_

#include "wx/defs.h"
#include "wx/dc.h"
#include "wx/bitmap.h"

#if wxUSE_GUI

//-----------------------------------------------------------------------------
// wxMemoryDC
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxMemoryDC: public wxDC
{
public:
    wxMemoryDC();
    wxMemoryDC( wxBitmap& bitmap );
    wxMemoryDC( wxDC *dc );

    // select the given bitmap to draw on it
    void SelectObject(wxBitmap& bmp);

    // select the given bitmap for read-only
    void SelectObjectAsSource(const wxBitmap& bmp);

    // get selected bitmap
    const wxBitmap& GetSelectedBitmap() const;
    wxBitmap& GetSelectedBitmap();

private:
    wxDECLARE_DYNAMIC_CLASS(wxMemoryDC);
};

#endif

#endif
    // _WX_DCMEMORY_H_BASE_
