/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/accel.h
// Purpose:     wxAcceleratorTable class
// Author:      Julian Smart
// Created:     31/7/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ACCEL_H_
#define _WX_ACCEL_H_

#include "wx/platform.h"

#if wxUSE_GUI

class WXDLLIMPEXP_FWD_CORE wxWindow;

// ----------------------------------------------------------------------------
// the accel table has all accelerators for a given window or menu
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxAcceleratorTable : public wxObject
{
public:
    // default ctor
    wxAcceleratorTable() { }

    // load from .rc resource (Windows specific)
    wxAcceleratorTable(const wxString& resource);

    // initialize from array
    wxAcceleratorTable(int n, const wxAcceleratorEntry entries[]);

    bool Ok() const { return IsOk(); }
    bool IsOk() const;
    void SetHACCEL(WXHACCEL hAccel);
    WXHACCEL GetHACCEL() const;

    // translate the accelerator, return true if done
    bool Translate(wxWindow *window, WXMSG *msg) const;

private:
    wxDECLARE_DYNAMIC_CLASS(wxAcceleratorTable);
};

#endif

#endif
    // _WX_ACCEL_H_
