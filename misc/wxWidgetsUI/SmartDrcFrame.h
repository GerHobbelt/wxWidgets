#pragma once

#include "wx/aui/aui.h"
#include "BoardViewer.h"

class cSmartDrcFrame
   : public wxDocParentFrameAny<wxAuiMDIParentFrame>
{
   enum {
      ID_CreateTree = wxID_HIGHEST + 1,
      ID_CustomizeToolbar,
   };

   using Super = wxDocParentFrameAny<wxAuiMDIParentFrame>;

public:
   enum SBField { Field_Text, Field_Elapsed, Field_Coord, Field_Max };

   cSmartDrcFrame(wxDocManager* docmgr, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition,
      const wxSize &size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);

   virtual ~cSmartDrcFrame()
   {
   }

   void SetStatusText(SBField field, const wxString& text);

private:
   wxStatusBar* m_status_bar;

   wxDECLARE_EVENT_TABLE();
};
