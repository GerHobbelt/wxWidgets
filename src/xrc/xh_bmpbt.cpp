/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_bmpbt.cpp
// Purpose:     XRC resource for bitmap buttons
// Author:      Brian Gavin
// Created:     2000/09/09
// Copyright:   (c) 2000 Brian Gavin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_XRC && wxUSE_BMPBUTTON

#include "wx/xrc/xh_bmpbt.h"

#ifndef WX_PRECOMP
    #include "wx/bmpbuttn.h"
#endif

wxIMPLEMENT_DYNAMIC_CLASS(wxBitmapButtonXmlHandler, wxXmlResourceHandler);

wxBitmapButtonXmlHandler::wxBitmapButtonXmlHandler()
: wxXmlResourceHandler()
{
    XRC_ADD_STYLE(wxBU_AUTODRAW);
    XRC_ADD_STYLE(wxBU_LEFT);
    XRC_ADD_STYLE(wxBU_RIGHT);
    XRC_ADD_STYLE(wxBU_TOP);
    XRC_ADD_STYLE(wxBU_BOTTOM);
    XRC_ADD_STYLE(wxBU_EXACTFIT);
    AddWindowStyles();
}

wxObject *wxBitmapButtonXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(button, wxBitmapButton)

    if ( GetBool("close", 0) )
    {
        button->CreateCloseButton(m_parentAsWindow,
                                  GetID(),
                                  GetName());
    }
    else
    {
        button->Create(m_parentAsWindow,
                       GetID(),
                       GetBitmapOrBitmaps(wxT("bitmap"), wxT("bitmaps"), wxART_BUTTON),
                       GetPosition(), GetSize(),
                       GetStyle(wxT("style")),
                       wxDefaultValidator,
                       GetName());
    }

    if (GetBool(wxT("default"), 0))
        button->SetDefault();
    SetupWindow(button);

    wxBitmapBundle selectedBitmaps = GetBitmapOrBitmaps(wxT("selected"), wxT("selected-bitmaps"));
    if (selectedBitmaps.IsOk())
        button->SetBitmapPressed(selectedBitmaps);
    wxBitmapBundle focusBitmaps = GetBitmapOrBitmaps(wxT("focus"), wxT("focus-bitmaps"));
    if (focusBitmaps.IsOk())
        button->SetBitmapFocus(focusBitmaps);
    wxBitmapBundle disabledBitmaps = GetBitmapOrBitmaps(wxT("disabled"), wxT("disabled-bitmaps"));
    if (disabledBitmaps.IsOk())
        button->SetBitmapDisabled(disabledBitmaps);
    wxBitmapBundle hoverBitmaps = GetBitmapOrBitmaps(wxT("hover"), wxT("hover-bitmaps"));
    if (hoverBitmaps.IsOk())
        button->SetBitmapCurrent(hoverBitmaps);

    return button;
}

bool wxBitmapButtonXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxT("wxBitmapButton"));
}

#endif // wxUSE_XRC && wxUSE_BMPBUTTON
