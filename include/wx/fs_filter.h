/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fs_filter.h
// Purpose:     Filter file system handler
// Author:      Mike Wetherell
// Copyright:   (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FS_FILTER_H_
#define _WX_FS_FILTER_H_

#include "wx/defs.h"

#if wxUSE_FILESYSTEM

#include "wx/filesys.h"

//---------------------------------------------------------------------------
// wxFilterFSHandler
//---------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxFilterFSHandler : public wxFileSystemHandler
{
public:
    wxFilterFSHandler()  = default;
    ~wxFilterFSHandler() override = default;

    wxFilterFSHandler(const wxFilterFSHandler&) = delete;
	wxFilterFSHandler& operator=(const wxFilterFSHandler&) = delete;

    bool CanOpen(const wxString& location) override;
    wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location) override;

    wxString FindFirst(const wxString& spec, int flags = 0) override;
    wxString FindNext() override;
};

#endif // wxUSE_FILESYSTEM

#endif // _WX_FS_FILTER_H_
