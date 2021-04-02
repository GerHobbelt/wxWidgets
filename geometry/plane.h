#pragma once

#include "geom_type_desc.h"

struct cPlane
   : public iPlane
{
   int m_id = 0;
   string m_name;

   map<ObjectType, cGeomTypeDesc> m_shape_types;

   cPlane(int a_id, const char* a_name)
      : m_id(a_id)
      , m_name(a_name)
   {
   }
   ~cPlane()
   {
   }

   int id() const override
   {
      return m_id;
   }

   const char* name() const override
   {
      return m_name.c_str();
   }

   void add_shape(iShape* ps, ObjectType type) override
   {
      m_shape_types[type].add_shape(ps);
   }
   void remove_shape(iShape* ps) override
   {
   }
   cShapeIter shapes(const cRect& bounds, ObjectType type, RetrieveOptions opt = RetrieveOptions::shape) const override
   {
      auto it = m_shape_types.find(type);
      if (it != m_shape_types.end()) {
         iShapeIter* res = nullptr;
         if (const_cast<cGeomTypeDesc&>(it->second).shapes(&res, bounds, opt)) {
            return cShapeIter(res);
         }
      }
      return cShapeIter();
   }
   cRect bounds() override
   {
      constexpr auto inf = numeric_limits<coord_t>::infinity();
      coord_t minX = inf, minY = inf, maxX = -inf, maxY = -inf;
      for (auto& [type, desc] : m_shape_types) {
         desc.commit();
         minX = min(minX, desc.m_index.minX());
         minY = min(minY, desc.m_index.minY());
         maxX = max(maxX, desc.m_index.maxX());
         maxY = max(maxY, desc.m_index.maxY());
      }
      return { minX, minY, maxX, maxY };
   }
};
