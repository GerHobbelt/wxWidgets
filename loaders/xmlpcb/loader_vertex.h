#pragma once

#include "loader_base.h"

struct cLoaderVertex : public cLoaderBase
{
   bool m_arc = false;
   geom::cPoint m_point;
   geom::coord_t m_radius;

   cLoaderVertex()
      : cLoaderBase(nullptr)
   {
   }
   cLoaderVertex(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::x:
            m_point.m_x = get_coord(value);
            break;
         case eKeyword::y:
            m_point.m_y = get_coord(value);
            break;
         case eKeyword::r:
            m_arc = true;
            m_radius = get_coord(value);
            break;
      }
   }

   void Delete() override
   {
   }
};
