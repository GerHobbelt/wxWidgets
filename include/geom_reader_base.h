#pragma once

#include "stdlib.h"
#include "geom_model.h"

class cGeomReaderBase
{
public:
   double m_scale = 1;
   geom::iEngine* m_ge = nullptr;
   std::map<std::string, std::unique_ptr<geom::iPlane>> m_planes;

   double get_coord(const char * text_val) const
   {
      return atof(text_val) * m_scale;
   }

   geom::iShape* add_to_plane(geom::iShape* ps, const char* layer, geom::ObjectType id)
   {
      auto it = m_planes.find(layer);
      if (it == m_planes.end()) {
         it = m_planes.emplace(layer, m_ge->create_plane(0, layer)).first; // 1-based layer numbering 
      }
      geom::iPlane* plane = it->second.get();
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

   void commit()
   {
      if (m_current_hole) {
         m_current_hole->commit();
      }
      if (m_current_shape) {
         m_current_shape->commit();
      }
      m_current_hole_id = -1;
      m_current_shape_id = -1;

      for (auto& [id, plane] : m_planes) {
         plane->bounds(); // trigger commit
      }
   }
};
