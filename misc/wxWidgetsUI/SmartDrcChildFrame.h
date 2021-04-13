#pragma once

#include "wx/aui/tabmdi.h"

#include "BoardViewer.h"

using cFrameAny = wxDocChildFrameAny<wxAuiMDIChildFrame, wxAuiMDIParentFrame>;

class cSmartrcChildFrame : public cFrameAny
{
   std::unique_ptr<cDrawArea> m_draw_area;
   std::unique_ptr<cSmartDrcProjectExplorer> m_project_explorer;

   wxAuiManager m_dockmgr;

   struct cPersist;

   cPersist *m_persist = nullptr;
   friend wxPersistentObject *wxCreatePersistentObject(cSmartrcChildFrame *obj);

   wxDECLARE_EVENT_TABLE();

public:
   cSmartrcChildFrame(wxDocument *doc, wxView *view, wxAuiMDIParentFrame *parent, wxWindowID id, const wxString &title,
      const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE,
      const wxString &name = wxASCII_STR(wxFrameNameStr));

   ~cSmartrcChildFrame();

   void Init(cPcbDesignDocument *doc);
};

wxPersistentObject *wxCreatePersistentObject(cSmartrcChildFrame *obj);
