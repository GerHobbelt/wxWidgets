#pragma once

struct cLoaderBoardOutline : public cLoaderBase
{
   cLoaderBoardOutline(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      m_ldr->m_board = m_ldr->m_db->createBoardRegion();
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      m_ldr->m_loader_stack.push_back(new cLoaderShape(m_ldr, atts, m_ldr->m_board, eObjId::BoardRegion));
   }
};
