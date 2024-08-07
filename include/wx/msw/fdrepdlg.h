/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/fdrepdlg.h
// Purpose:     wxFindReplaceDialog class
// Author:      Markus Greither
// Modified by: 31.07.01: VZ: integrated into wxWidgets
// Created:     23/03/2001
// Copyright:   (c) Markus Greither
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_FDREPDLG_H_
#define _WX_MSW_FDREPDLG_H_

#include "wx/platform.h"

#if wxUSE_GUI

// ----------------------------------------------------------------------------
// wxFindReplaceDialog: dialog for searching / replacing text
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxFindReplaceDialog : public wxFindReplaceDialogBase
{
public:
    // ctors and such
    wxFindReplaceDialog() { Init(); }
    wxFindReplaceDialog(wxWindow *parent,
                        wxFindReplaceData *data,
                        const wxString &title,
                        int style = 0);

    bool Create(wxWindow *parent,
                wxFindReplaceData *data,
                const wxString &title,
                int style = 0);

    virtual ~wxFindReplaceDialog();

    // implementation only from now on

    wxFindReplaceDialogImpl *GetImpl() const { return m_impl; }

    // override some base class virtuals
    virtual bool Show(bool show = true) override;
    virtual void SetTitle( const wxString& title) override;
    virtual wxString GetTitle() const override;

    virtual bool MSWProcessMessage(WXMSG* pMsg) override;

protected:
    virtual void DoGetSize(int *width, int *height) const override;
    virtual void DoGetClientSize(int *width, int *height) const override;
    virtual void DoSetSize(int x, int y,
                           int width, int height,
                           int sizeFlags = wxSIZE_AUTO) override;

    void Init();

    wxString                m_title;

    wxFindReplaceDialogImpl *m_impl;

    wxDECLARE_DYNAMIC_CLASS(wxFindReplaceDialog);
    wxDECLARE_NO_COPY_CLASS(wxFindReplaceDialog);
};

#endif

#endif // _WX_MSW_FDREPDLG_H_
