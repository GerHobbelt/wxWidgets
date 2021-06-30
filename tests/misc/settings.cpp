///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/settings.cpp
// Purpose:     test wxSettings
// Author:      Francesco Montorsi
// Created:     2009-03-24
// Copyright:   (c) 2009 Francesco Montorsi
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "doctest.h"

#include "testprec.h"


#include "wx/settings.h"
#include "wx/fontenum.h"
#include "wx/brush.h"
#include "wx/pen.h"

TEST_CASE("Get colour")
{
    for (unsigned int i=wxSYS_COLOUR_SCROLLBAR; i < wxSYS_COLOUR_MAX; i++)
        CHECK( wxSystemSettings::GetColour((wxSystemColour)i).IsOk() );
}

TEST_CASE("Get font")
{
    const wxSystemFont ids[] =
    {
        wxSYS_OEM_FIXED_FONT,
        wxSYS_ANSI_FIXED_FONT,
        wxSYS_ANSI_VAR_FONT,
        wxSYS_SYSTEM_FONT,
        wxSYS_DEVICE_DEFAULT_FONT,
        wxSYS_SYSTEM_FIXED_FONT,
        wxSYS_DEFAULT_GUI_FONT
    };

    for (unsigned int i=0; i < WXSIZEOF(ids); i++)
    {
        const wxFont& font = wxSystemSettings::GetFont(ids[i]);
        CHECK( font.IsOk() );
        CHECK( wxFontEnumerator::IsValidFacename(font.GetFaceName()) );
    }
}

TEST_CASE("Get global colours")
{
    wxColour col[] =
    {
        *wxBLACK,
        *wxBLUE,
        *wxCYAN,
        *wxGREEN,
        *wxLIGHT_GREY,
        *wxRED,
        *wxWHITE
    };

    for (unsigned int i=0; i < WXSIZEOF(col); i++)
        CHECK( col[i].IsOk() );
}

TEST_CASE("Get global fonts")
{
    const wxFont font[] =
    {
        *wxNORMAL_FONT,
        *wxSMALL_FONT,
        *wxITALIC_FONT,
        *wxSWISS_FONT
    };

    for (unsigned int i=0; i < WXSIZEOF(font); i++)
    {
        CHECK( font[i].IsOk() );

        const wxString facename = font[i].GetFaceName();
        if ( !facename.empty() )
        {
            CHECK_MESSAGE(
                wxFontEnumerator::IsValidFacename(facename),
                ("font #%u: facename \"%s\" is invalid", i, facename)
            );
        }
    }
}

TEST_CASE("Get global brushes")
{
    wxBrush brush[] =
    {
        *wxBLACK_BRUSH,
        *wxBLUE_BRUSH,
        *wxCYAN_BRUSH,
        *wxGREEN_BRUSH,
        *wxGREY_BRUSH,
        *wxLIGHT_GREY_BRUSH,
        *wxMEDIUM_GREY_BRUSH,
        *wxRED_BRUSH,
        *wxTRANSPARENT_BRUSH,
        *wxWHITE_BRUSH
    };

    for (unsigned int i=0; i < WXSIZEOF(brush); i++)
        CHECK( brush[i].IsOk() );
}

TEST_CASE("Get global pens")
{
    wxPen pen[] =
    {
        *wxBLACK_DASHED_PEN,
        *wxBLACK_PEN,
        *wxBLUE_PEN,
        *wxCYAN_PEN,
        *wxGREEN_PEN,
        *wxGREY_PEN,
        *wxLIGHT_GREY_PEN,
        *wxMEDIUM_GREY_PEN,
        *wxRED_PEN,
        *wxTRANSPARENT_PEN,
        *wxWHITE_PEN
    };

    for (unsigned int i=0; i < WXSIZEOF(pen); i++)
        CHECK( pen[i].IsOk() );
}
