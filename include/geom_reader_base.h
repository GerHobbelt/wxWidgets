#pragma once

#include "stdlib.h"
#include "geom_model.h"

class cGeomReaderBase
{
protected:
   double m_scale = 1;
   geom::iEngine* m_ge = nullptr;

   double get_coord(const char * text_val) const
   {
      return atof(text_val) * m_scale;
   }

   geom::iShape* add_to_plane(geom::iShape* ps, const char* layer, geom::ObjectType id)
   {
      geom::iPlane* plane = m_ge->plane(layer);
      if (!plane) {
         plane = m_ge->create_plane(0, layer);
      }
      plane->add_shape(ps, id);
      return ps;
   }

   int m_current_shape_id = -1, m_current_hole_id = -1;
   geom::iPlane* m_current_plane = nullptr;
   geom::iShape* m_current_shape = nullptr;
   geom::iShape* m_current_hole = nullptr;

public:
   cGeomReaderBase(geom::iEngine* ge = nullptr)
      : m_ge(ge)
   {
   }
};
