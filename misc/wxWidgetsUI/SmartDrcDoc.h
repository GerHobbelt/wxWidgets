#pragma once

#include "wx/docview.h"

#include "pcb_loader.h"
#include "smartdrc.h"

class cPcbDesignDocument : public wxDocument
{
public:
   cPcbDesignDocument()
   {
   }

   bool OnCreate(const wxString& path, long flags) override;

   geom::cRect GetWorldRect()
   {
      auto db = database();
      return geom::cRect(db->m_x1, db->m_y1, db->m_x2, db->m_y2);
   }
   cDatabase *database()
   {
      return m_db ? m_db->database() : nullptr;
   }
   iDbHolder* db_holder()
   {
      return m_db.get();
   }

protected:
   std::unique_ptr<iDbHolder> m_db;

   wxDECLARE_DYNAMIC_CLASS(cPcbDesignDocument);
};
