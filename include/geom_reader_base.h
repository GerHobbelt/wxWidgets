#pragma once

#include "stdlib.h"
#include "geom_model.h"

#include "fast_atod.h"
#include "../geometry/geom_engine_base.h"

class cGeomReaderBase
{
public:
   cGeomEngineBase* m_ge = nullptr;
   std::map<std::string, cPlaneBase*> m_planes;

   double get_coord(const char * text_val) const
   {
      return fast_atod(text_val);
   }

   cGeomImplBase* add_to_plane(cGeomImplBase* ps, const char* layer, geom::ObjectType id)
   {
      auto it = m_planes.find(layer);
      if (it == m_planes.end()) {
         it = m_planes.emplace(layer, m_ge->create_plane(0, layer)).first; // 1-based layer numbering 
      }
      cPlaneBase* plane = it->second;
      plane->add_shape(ps, id);
      return ps;
   }

   int m_current_shape_id = -1, m_current_hole_id = -1;
   cPlaneBase *m_current_plane = nullptr;
   cShapeImpl *m_current_shape = nullptr;
   cShapeImpl *m_current_hole = nullptr;

public:
   cGeomReaderBase(cGeomEngineBase* ge = nullptr)
      : m_ge(ge)
   {
   }

   void commit()
   {
      m_current_hole_id = -1;
      m_current_shape_id = -1;

      for (auto& [id, plane] : m_planes) {
         plane->bounds(); // trigger commit
      }
   }
};
