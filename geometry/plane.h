#pragma once

#include "geom_type_desc.h"

struct cPlaneBase
{
   int m_id = 0;
   shm::string m_name;

   using shape_types_t = shm::map<ObjectType, cGeomTypeDesc>;
   shape_types_t m_shape_types;

   cPlaneBase(int a_id, const char* a_name)
      : m_id(a_id)
      , m_name(a_name)
      , m_shape_types(shm::alloc<shape_types_t::value_type>())
   {
   }
   ~cPlaneBase()
   {
   }

   int id() const
   {
      return m_id;
   }

   const char* name() const
   {
      return m_name.c_str();
   }

   cGeomTypeDesc* get_type_desc(ObjectType type, bool create = false)
   {
      auto it = m_shape_types.find(type);
      if (create && it == m_shape_types.end()) {
         it = m_shape_types.emplace(type, cGeomTypeDesc()).first;
      }
      return it == m_shape_types.end() ? nullptr : &it->second;
   }
   cRect bounds()
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

   void clear()
   {
      //TBD
   }
};

struct cPlane
   : public iPlane
{
   cPlaneBase* m_pPlane;

   cPlane(cPlaneBase* pPlane)
      : m_pPlane(pPlane)
   {
   }
   ~cPlane()
   {
   }

   int id() const override
   {
      return m_pPlane->m_id;
   }

   const char* name() const override
   {
      return m_pPlane->m_name.c_str();
   }

   void add_shape(iShape* ps, ObjectType type) override
   {
      auto type_desc = m_pPlane->get_type_desc(type, true);
      type_desc->add_shape(ps);
   }
   void remove_shape(iShape* ps) override
   {
   }
   cShapeIter shapes(const cRect& bounds, ObjectType type, RetrieveOptions opt = RetrieveOptions::shape) const override
   {
      if (auto type_desc = m_pPlane->get_type_desc(type)) {
         iShapeIter* res = nullptr;
         if (type_desc->shapes(&res, bounds, opt)) {
            return cShapeIter(res);
         }
      }
      return cShapeIter();
   }
   cRect bounds() override
   {
      return m_pPlane->bounds();
   }
};
