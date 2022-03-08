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

#include "wx/debugheap.h"
#include "wx/toolbook.h"
#include "wx/imaglist.h"

#include "wx/xml/xml.h"


struct wxToolbookPageInfo
{
    wxWindow* window = NULL;
    wxString label;
    bool selected = false;
    int imageId = -1;
};



// DbgHeap struct ripped from MSVC2019 MSVCRT sources:

// The size of the no-man's land used in unaligned and aligned allocations:
static size_t const no_mans_land_size = 4;
static size_t const align_gap_size = sizeof(void*);

struct _CrtMemBlockHeader
{
	_CrtMemBlockHeader* _block_header_next;
	_CrtMemBlockHeader* _block_header_prev;
	char const* _file_name;
	int                 _line_number;

	int                 _block_use;
	size_t              _data_size;

	long                _request_number;
	unsigned char       _gap[no_mans_land_size];

	// Followed by:
	// unsigned char    _data[_data_size];
	// unsigned char    _another_gap[no_mans_land_size];
};

int CrtIsMemoryBlock(void const* ptr)
{
	long requestNumber = 0;
	char* fileName;
	int lineNumber;
	int size = 0;
	if (ptr != NULL)
	{
		struct _CrtMemBlockHeader* info = (struct _CrtMemBlockHeader*)ptr;
		info--;
		size = info->_data_size;
	}
	int rv = _CrtIsMemoryBlock(ptr, size, &requestNumber, &fileName, &lineNumber);
	return rv;
}



wxIMPLEMENT_DYNAMIC_CLASS(wxToolbookXmlHandler, wxXmlResourceHandler);

wxToolbookXmlHandler::wxToolbookXmlHandler()
                    : m_toolbook(NULL)
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


wxToolbookXmlHandler::~wxToolbookXmlHandler()
{
	for (size_t i = 0; i < m_bookPages.size(); ++i)
	{
		const wxToolbookPageInfo& info = m_bookPages[i];
		wxCHECK2(CrtIsMemoryBlock(info.window), break);
		info.window->DoNotB0rkOnDelete();
	}
	m_bookImages.clear();
	m_bookPages.clear();
}


wxObject *wxToolbookXmlHandler::DoCreateResource()
{
    if (m_class == wxT("toolbookpage"))
    {
        return DoCreatePage(m_toolbook);
    }
    else
    {
        XRC_MAKE_INSTANCE(nb, wxToolbook)

        nb->Create( m_parentAsWindow,
                    GetID(),
                    GetPosition(), GetSize(),
                    GetStyle(wxT("style")),
                    GetName() );

        wxToolbook *old_par = m_toolbook;
        m_toolbook = nb;

        DoCreatePages(m_toolbook);

        m_toolbook = old_par;

        return nb;
    }
}

bool wxToolbookXmlHandler::CanHandle(wxXmlNode *node)
{
    return ((!IsInside() && IsOfClass(node, wxT("wxToolbook"))) ||
            (IsInside() && IsOfClass(node, wxT("toolbookpage"))));
}

#endif // wxUSE_XRC && wxUSE_TOOLBOOK
