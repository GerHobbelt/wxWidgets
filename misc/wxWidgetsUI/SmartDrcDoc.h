#pragma once

#include "wx/docview.h"

#include "pcb_loader.h"
#include "database.h"

class cPcbDesignDocument : public wxDocument
{
public:
   cPcbDesignDocument()
   {
   }

   bool OnCreate(const wxString& path, long flags) override;

   geom::cRect GetWorldRect()
   {
      return geom::cRect(m_db->m_x1, m_db->m_y1, m_db->m_x2, m_db->m_y2);
   }
   cDatabase *database()
   {
      return m_db;
   }

protected:
   cDatabase* m_db = nullptr;

   wxDECLARE_DYNAMIC_CLASS(cPcbDesignDocument);
};
