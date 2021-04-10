#pragma once

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
            m_point.m_x = fast_atod(value);
            break;
         case eKeyword::y:
            m_point.m_y = fast_atod(value);
            break;
         case eKeyword::r:
            m_arc = true;
            m_radius = fast_atod(value);
            break;
      }
   }

   void Delete() override
   {
   }
};
