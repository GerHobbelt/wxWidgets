#pragma once

#undef max
#undef min

#include "plane.h"

#include "shared_memory.h"

struct cGeomEngineBase
{
   using planes_t = shm::vector<shm::unique_offset_ptr<cPlaneBase>>;
   planes_t m_planes;

   using plane_dict_t = shm::string_map<shm::offset_ptr<cPlaneBase>>;
   plane_dict_t m_plane_dict;

   cGeomEngineBase()
      : m_plane_dict(shm::alloc<plane_dict_t::value_type>())
      , m_planes(shm::alloc<planes_t::value_type>())
   {
   }

   cPlaneBase *create_plane(int id, const shm::string::value_type *name)
   {
      auto pPlane = shm::construct<cPlaneBase>(id, name);
      m_planes.push_back(pPlane);
      auto &p = m_planes.back();
      m_plane_dict.emplace(p->m_name, &*p);
      return pPlane;
   }
   cPlaneBase *plane(const char *name)
   {
      auto it = m_plane_dict.find(name);
      if (it != m_plane_dict.end()) {
         return &*it->second;
      }
      return nullptr;
   }
   cPlaneBase *plane(size_t id)
   {
      for (auto &p: m_planes) {
         if (p->m_id == id) {
            return &*p;
         }
      }
      return nullptr;
   }
   size_t planes() const
   {
      return m_planes.size();
   }

   void clear()
   {
      for (auto &plane: m_planes) {
         plane->clear();
      }
      m_plane_dict.clear();
      m_planes.clear();
   }
};
