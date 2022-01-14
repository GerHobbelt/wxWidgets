/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_toolbk.cpp
// Purpose:     XRC resource for wxToolbook
// Author:      Andrea Zanellato
// Created:     2009/12/12
// Copyright:   (c) 2010 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_XRC && wxUSE_TOOLBOOK

#include "wx/xrc/xh_toolbk.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/sizer.h"
#endif

#include "wx/toolbook.h"
#include "wx/imaglist.h"

#include "wx/xml/xml.h"


struct ToolbookPageInfo
{
    wxWindow* window = NULL;
    wxString label;
    bool selected = false;
    int image_id = -1;
};


wxIMPLEMENT_DYNAMIC_CLASS(wxToolbookXmlHandler, wxXmlResourceHandler);

wxToolbookXmlHandler::wxToolbookXmlHandler()
                     :wxXmlResourceHandler(),
                      m_isInside(false),
                      m_toolbook(NULL)
{
    XRC_ADD_STYLE(wxBK_DEFAULT);
    XRC_ADD_STYLE(wxBK_TOP);
    XRC_ADD_STYLE(wxBK_BOTTOM);
    XRC_ADD_STYLE(wxBK_LEFT);
    XRC_ADD_STYLE(wxBK_RIGHT);

    XRC_ADD_STYLE(wxTBK_BUTTONBAR);
    XRC_ADD_STYLE(wxTBK_HORZ_LAYOUT);

    AddWindowStyles();
}

wxObject *wxToolbookXmlHandler::DoCreateResource()
{
    if (m_class == wxT("toolbookpage"))
    {
        wxXmlNode *n = GetParamNode(wxT("object"));

        if ( !n )
            n = GetParamNode(wxT("object_ref"));

        if (n)
        {
            bool old_ins = m_isInside;
            m_isInside = false;
            wxObject *item = CreateResFromNode(n, m_toolbook, NULL);
            m_isInside = old_ins;
            ToolbookPageInfo next_page;
            next_page.window = wxDynamicCast(item, wxWindow);

            if (next_page.window)
            {
                next_page.image_id = -1;

                if ( HasParam(wxT("image")) )
                {
                    if ( m_toolbook->GetImageList() )
                    {
                        next_page.image_id = (int)GetLong(wxT("image"));
                    }
                    else // image without image list?
                    {
                        ReportError(n, "image can only be used in conjunction "
                                       "with imagelist");
                    }
                }
                else
                {
                    wxBitmapBundle bb = GetBitmapOrBitmaps();
                    if ( bb.IsOk() )
                    {
                        m_images.push_back(bb);
                        next_page.image_id = m_images.size() - 1;
                    }
                }

                next_page.label = GetText(wxT("label"));
                next_page.selected = GetBool(wxT("selected"));
                m_pages.push_back(next_page);
            }
            else
            {
                ReportError(n, "toolbookpage child must be a window");
            }
            return next_page.window;
        }
        else
        {
            ReportError("toolbookpage must have a window child");
            return NULL;
        }
    }

    else
    {
        XRC_MAKE_INSTANCE(nb, wxToolbook)

        nb->Create( m_parentAsWindow,
                    GetID(),
                    GetPosition(), GetSize(),
                    GetStyle(wxT("style")),
                    GetName() );

        wxImageList *imagelist = GetImageList();
        bool has_imagelist = false;
        if ( imagelist )
        {
            has_imagelist = true;
            nb->AssignImageList(imagelist);
        }

        wxToolbook *old_par = m_toolbook;
        m_toolbook = nb;
        bool old_ins = m_isInside;
        m_isInside = true;
        CreateChildren(m_toolbook, true/*only this handler*/);

        if ( !m_images.empty() )
        {
            if ( has_imagelist )
                ReportError("toolbook can't have an imagelist and use bitmaps in toolbookpages at the same time");
            else
                m_toolbook->SetImages(m_images);
        }

        // insert pages
        for (size_t i = 0; i < m_pages.size(); ++i)
        {
            m_toolbook->AddPage(m_pages[i].window, m_pages[i].label,
                m_pages[i].selected, m_pages[i].image_id );
        }

        m_isInside = old_ins;
        m_toolbook = old_par;

        return nb;
    }
}

bool wxToolbookXmlHandler::CanHandle(wxXmlNode *node)
{
    return ((!m_isInside && IsOfClass(node, wxT("wxToolbook"))) ||
            (m_isInside && IsOfClass(node, wxT("toolbookpage"))));
}

#endif // wxUSE_XRC && wxUSE_TOOLBOOK
