/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/button.h
// Purpose:     wxButton class
// Author:      Julian Smart
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_BUTTON_H_
#define _WX_MSW_BUTTON_H_

#include "wx/platform.h"

#if wxUSE_GUI

// ----------------------------------------------------------------------------
// Pushbutton
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxButton : public wxButtonBase
{
public:
    wxButton() { Init(); }
    wxButton(wxWindow *parent,
             wxWindowID id,
             const wxString& label = wxEmptyString,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = 0,
             const wxValidator& validator = wxDefaultValidator,
             const wxString& name = wxASCII_STR(wxButtonNameStr))
    {
        Init();

        Create(parent, id, label, pos, size, style, validator, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& label = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxButtonNameStr));

    virtual ~wxButton();

    virtual wxWindow *SetDefault() override;

    // implementation from now on
    virtual void Command(wxCommandEvent& event) override;
    virtual WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override;
    virtual bool MSWCommand(WXUINT param, WXWORD id) override;

    virtual WXDWORD MSWGetStyle(long style, WXDWORD *exstyle) const override;

protected:
    // send a notification event, return true if processed
    bool SendClickEvent();

    // default button handling
    void SetTmpDefault();
    void UnsetTmpDefault();

    // set or unset BS_DEFPUSHBUTTON style
    static void SetDefaultStyle(wxButton *btn, bool on);

    virtual bool DoGetAuthNeeded() const override;
    virtual void DoSetAuthNeeded(bool show) override;

    // true if the UAC symbol is shown
    bool m_authNeeded;

private:
    void Init()
    {
        m_authNeeded = false;
    }

    void OnCharHook(wxKeyEvent& event);

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxButton);
};

#endif

#endif // _WX_MSW_BUTTON_H_
