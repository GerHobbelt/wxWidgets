/////////////////////////////////////////////////////////////////////////////
// Name:        src/osx/carbon/gdiobj.cpp
// Purpose:     wxGDIObject class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

// source is included by gdicmn.cpp: this will discover that it is and act accordingly.
#if defined(MAIN_STOCKGDI_CLASS_NAME)

#include "wx/gdiobj.h"

#ifndef WX_PRECOMP
    #include "wx/gdicmn.h"
    #include "wx/module.h"
#endif

#include "wx/link.h"
#include "wx/osx/private.h"
#include "wx/font.h"

// override for proper init in gdicmn.cpp:
#undef MAIN_STOCKGDI_CLASS_NAME
#define MAIN_STOCKGDI_CLASS_NAME wxStockGDIMac


class wxStockGDIMac: public wxStockGDI
{
public:
    virtual const wxFont* GetFont(Item item) override;

private:
    typedef wxStockGDI super;
};

const wxFont* wxStockGDIMac::GetFont(Item item)
{
    wxFont* font = static_cast<wxFont*>(ms_stockObject[item]);
    if (font == nullptr)
    {
        switch (item)
        {
        case FONT_NORMAL:
            font = new wxFont(wxOSX_SYSTEM_FONT_NORMAL);
            break;
        case FONT_SMALL:
            font = new wxFont(wxOSX_SYSTEM_FONT_SMALL);
            break;
        default:
            font = const_cast<wxFont*>(super::GetFont(item));
            break;
        }
        ms_stockObject[item] = font;
    }
    return font;
}

#endif
