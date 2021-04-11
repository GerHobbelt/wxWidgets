#pragma once

#include "loader_base.h"

struct cLoaderBoardOutline : public cLoaderBase
{
   cLoaderBoardOutline(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      m_ldr->m_board = m_ldr->m_db->createBoardRegion();
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      auto shape = new cLoaderShape(m_ldr, atts, this, eObjId::BoardRegion);
      shape->m_filled = false;
      m_ldr->m_loader_stack.push_back(shape);
   }
   void OnEndElement(const cChar *name) override
   {
      add_shapes(m_ldr->m_board);
      cLoaderBase::OnEndElement(name);
   }
};
