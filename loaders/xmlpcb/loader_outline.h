#pragma once

#include "loader_base.h"

struct cLoaderOutline : public cLoaderBase
{
   cOutline *outline = nullptr;
   eObjId type;

   cLoaderOutline(cXmlPcbSaxLoader *ldr, const cChar **atts, cOutline *ol, eObjId geom_type)
      : cLoaderBase(ldr)
      , outline(ol)
      , type(geom_type)
   {
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      auto shape = new cLoaderShape(m_ldr, atts, this, type);
      shape->m_filled = false;
      m_ldr->m_loader_stack.push_back(shape);
   }
   void OnEndElement(const cChar *name) override
   {
      add_shapes(outline);
      cLoaderBase::OnEndElement(name);
   }
};
