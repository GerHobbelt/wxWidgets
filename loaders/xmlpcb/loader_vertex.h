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
      loadAttributes(atts, [this](eKeyword kw, const cChar *value) {
         switch (kw) {
            case eKeyword::x:
               m_point.m_x = atof(value);
               break;
            case eKeyword::y:
               m_point.m_y = atof(value);
               break;
            case eKeyword::r:
               m_arc = true;
               m_radius = atof(value);
               break;
         }
      });
   }

   void Delete() override
   {
   }
};
