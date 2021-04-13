
#pragma once

#include "wx/dataview.h"
#include "SmartDrcDoc.h"

class cSmartDrcProjectExplorer : public wxDataViewTreeCtrl
{
public:
   cSmartDrcProjectExplorer(cPcbDesignDocument* doc, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
      long style = wxDV_NO_HEADER | wxDV_ROW_LINES, const wxValidator& validator = wxDefaultValidator);

   void Init();

protected:
   cPcbDesignDocument* m_doc;
};
