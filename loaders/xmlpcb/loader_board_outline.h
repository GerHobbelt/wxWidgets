#pragma once

struct cLoaderBoardOutline : public cLoaderBase
{
   cBoard *board;

   cLoaderBoardOutline(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      board = m_ldr->m_db->createBoard();
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      m_ldr->m_loader_stack.push_back(new cLoaderShape(m_ldr, atts, board, eObjId::Board));
   }
};
