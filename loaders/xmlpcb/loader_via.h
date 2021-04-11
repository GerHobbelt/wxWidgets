#pragma once

#include "loader_pads.h"

struct cLoaderVia : public cLoaderPads
{
   cVia *via = nullptr;

   cLoaderVia(cXmlPcbSaxLoader *ldr, const cChar **atts, cVia *v)
      : cLoaderPads(ldr)
      , via(v)
   {
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::DrillSize:
            via->setDrillSize(get_coord(value));
            break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      via->setPosition(m_position.m_point);
      include(via, m_pads, cDbTraits::eRelId::Object_Pad);
      cLoaderPads::OnEndElement(name);
   }
};
