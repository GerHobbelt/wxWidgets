
#include "pch.h"

#include <forward_list>

#include "attachment_list.h"
#include "attachment_types.h"

#include "geom_type_desc.h"
#include "geom_impl.h"

#include "circle_impl.h"
#include "segment_impl.h"
#include "arc_segment_impl.h"
#include "rect_impl.h"
#include "shape_impl.h"

#include "plane.h"

struct cGeomEngine
   : public iEngine
{
   shm::string::allocator_type m_alloc;
   shm::vector<shm::unique_offset_ptr<cPlane>> m_planes;
   shm::string_map<shm::offset_ptr<cPlane>> m_plane_dict;

   cGeomEngine()
      : m_alloc(shm::mshm.get_segment_manager())
      , m_plane_dict(m_alloc)
      , m_planes(m_alloc)
   {
   }

   iPlane* create_plane(int id, const shm::string::value_type* name) override
   {
      auto pPlane = shm::construct<cPlane>(m_alloc, id, name, m_alloc);
      m_planes.push_back(pPlane);
      auto& p = m_planes.back();
      m_plane_dict.emplace(p->m_name, p.get());
      return p.get();
   }
   iPlane* plane(const char* name) override
   {
      auto it = m_plane_dict.find(name);
      if (it != m_plane_dict.end()) {
         return it->second.get();
      }
      return nullptr;
   }
   iPlane* plane(size_t id) override
   {
      for (auto& p : m_planes) {
         if (p->m_id == id) {
            return p.get();
         }
      }
      return nullptr;
   }
   size_t planes() const override
   {
      return m_planes.size();
   }

   void clear() override
   {
      m_plane_dict.clear();
      m_planes.clear();
   }

   template <typename T, typename ...Args>
   T* create_in_shmem(Args... args)
   {
      auto shared = m_alloc.allocate(sizeof T);
      return new (shared.get()) T(args...);
   }

   void create_circle(iShape** res, double x, double y, double radius, bool hole = false, bool filled = true, const char* tag = nullptr) override
   {
      *res = create_in_shmem<cCircleImpl>(hole, filled, x, y, radius PASS_TAG, m_alloc);
   }
   void create_segment(iShape** res, double x1, double y1, double x2, double y2, double width = 0, bool hole = false, bool filled = true, const char* tag = nullptr) override
   {
      *res = create_in_shmem<cSegmentImpl>(hole, filled, cPoint{ x1, y1 }, cPoint{ x2, y2 }, width PASS_TAG, m_alloc);
   }
   void create_arc_segment(iShape** res, coord_t x1, coord_t y1, coord_t x2, coord_t y2, coord_t center_x, coord_t center_y, coord_t r,
         coord_t width, bool hole, bool filled, const char* tag) override
   {
      *res = create_in_shmem<cArcSegmentImpl>(hole, filled, cPoint{ x1, y1 }, cPoint{ x2, y2 }, cPoint{center_x, center_y}, r, width PASS_TAG, m_alloc);
   }
   void create_arc_segment(iShape** res, coord_t x1, coord_t y1, coord_t x2, coord_t y2, double bulge, coord_t width, bool hole, bool filled, const char* tag) override
   {
      *res = create_in_shmem<cArcSegmentImpl>(hole, filled, cPoint{ x1, y1 }, cPoint{ x2, y2 }, bulge, width PASS_TAG, m_alloc);
   }
   void create_rectangle(iShape** res, double left, double bottom, double right, double top, bool hole = false, bool filled = true, const char* tag = nullptr) override
   {
      *res = create_in_shmem<cRectImpl>(hole, filled, left, bottom, right, top PASS_TAG, m_alloc);
   }
   void create_shape(iShape** res, bool hole = false, bool filled = true, const char* tag = nullptr) override
   {
      *res = create_in_shmem<cShapeImpl>(iPolygon::Type::polyline, hole, filled PASS_TAG, m_alloc);
   }
   void create_shape(iShape** res, iShape* ps) override
   {
   }

   void expand(iShape** res, const iShape* ps, double dist) override
   {
   }
   void invert(iShape** res, const iShape* ps) override
   {
   }

   void intersect(iShape** res, const iShape* ps1, const iShape* ps2) override
   {
   }
   void merge(iShape** res, const iShape* ps1, const iShape* ps2) override
   {
   }
   void cut(iShape** res, const iShape* ps1, const iShape* ps2) override
   {
   }

   bool distance(const iShape* ps1, const iShape* ps2, double& distance, double* x1 = 0, double* y1 = 0, double* x2 = 0, double* y2 = 0) override
   {
      return false;
   }
};

BOOST_SYMBOL_EXPORT
iEngine* GetGeomEngine()
{
   static bi::managed_unique_ptr<cGeomEngine, bi::managed_shared_memory>::type s_ge = bi::make_managed_unique_ptr(
      shm::construct<cGeomEngine>(
         shm::allocator<cGeomEngine>(
            shm::mshm.get_segment_manager()
         )
      ),
      shm::mshm
   );
   return s_ge.get().get();
}
