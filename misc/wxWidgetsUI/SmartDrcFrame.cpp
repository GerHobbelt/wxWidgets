
#include "pch.h"

#include "SmartDrcFrame.h"

wxBEGIN_EVENT_TABLE(cSmartDrcFrame, cSmartDrcFrame::Super)
wxEND_EVENT_TABLE()

cSmartDrcFrame::cSmartDrcFrame(wxDocManager* docmgr, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style)
   : Super(docmgr, nullptr, id, title, pos, size, style)
{
   int widths[Field_Max];
   widths[Field_Text] = -1;
   widths[Field_Elapsed] = 100;
   widths[Field_Coord] = 100;

   m_status_bar = new wxStatusBar(this);
   m_status_bar->SetFieldsCount(Field_Max);
   m_status_bar->SetStatusWidths(Field_Max, widths);
   SetStatusBar(m_status_bar);

   wxPersistentRegisterAndRestore(this, "MainFrame");
}

void cSmartDrcFrame::SetStatusText(SBField field, const wxString& text)
{
   m_status_bar->SetStatusText(text, field);
}
