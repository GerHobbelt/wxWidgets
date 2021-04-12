#pragma once

#include "geom_type_desc.h"
#include "circle_impl.h"
#include "segment_impl.h"
#include "arc_segment_impl.h"
#include "rect_impl.h"
#include "shape_impl.h"

struct cPlaneBase
{
   int m_id = 0;
   shm::string m_name;

   using shape_types_t = shm::map<geom::ObjectType, cGeomTypeDesc>;
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

   const cGeomTypeDesc* get_type_desc(geom::ObjectType type) const
   {
      if (m_shape_types.size()) {
         auto it = m_shape_types.find(type);
         if (it != m_shape_types.end()) {
            return &it->second;
         }
      }
      return nullptr;
   }
   const cGeomTypeDesc* get_type_desc(geom::ObjectType type, bool create = false)
   {
      if (auto retval = std::as_const(*this).get_type_desc(type)) {
         return retval;
      }
      if (create) {
         auto it = m_shape_types.emplace(type, cGeomTypeDesc()).first;
         return &it->second;
      }
      return nullptr;
   }
   geom::cRect bounds()
   {
      commit();

      constexpr auto inf = std::numeric_limits<geom::coord_t>::infinity();
      geom::coord_t minX = inf, minY = inf, maxX = -inf, maxY = -inf;
      for (auto& [type, desc] : m_shape_types) {
         minX = std::min(minX, desc.m_index.minX());
         minY = std::min(minY, desc.m_index.minY());
         maxX = std::max(maxX, desc.m_index.maxX());
         maxY = std::max(maxY, desc.m_index.maxY());
      }
      return { minX, minY, maxX, maxY };
   }

   void add_shape(cGeomImplBase *ps, geom::ObjectType type)
   {
      m_shape_types[type].m_shapes_temp.emplace_back(ps);
   }

   geom::cShapeIter shapes(const geom::cRect &bounds, geom::ObjectType type, geom::iPlane::RetrieveOptions opt = geom::iPlane::RetrieveOptions::shape) const
   {
      if (auto type_desc = get_type_desc(type)) {
         geom::iShapeIter *res = nullptr;
         if (type_desc->shapes(&res, bounds, opt)) {
            return geom::cShapeIter(res);
         }
      }
      return geom::cShapeIter();
   }

   void commit()
   {
      for (auto&& [type, desc] : m_shape_types) {
         if (auto size = desc.m_shapes_temp.size()) {
            desc.m_shapes.reserve(size);
            desc.m_shapes.clear();
            cGeomTypeDesc::cSpatialIndex new_index(size);
            for (auto &shape: desc.m_shapes_temp) {
               desc.m_shapes.push_back(shape);
               auto rectangle = [](const auto* shape) {
                  switch (shape->m_geom_type) {
                     case geom::iPolygon::Type::circle:
                        return ((cCircleImpl*)shape)->rectangle();
                     case geom::iPolygon::Type::segment:
                        return ((cSegmentImpl*)shape)->rectangle();
                     case geom::iPolygon::Type::arc_segment:
                        return ((cArcSegmentImpl*)shape)->rectangle();
                     case geom::iPolygon::Type::rectangle:
                        return ((cRectImpl*)shape)->rectangle();
                     case geom::iPolygon::Type::polyline:
                        return ((cShapeImpl*)shape)->rectangle();
                  }
                  return geom::cRect();
               };
               geom::cRect bounds = rectangle(&*shape);
               new_index.add(bounds.m_left, bounds.m_bottom, bounds.m_right, bounds.m_top);
            }
            new_index.finish();
            desc.m_index = std::move(new_index);
            desc.m_shapes_temp.clear();
         }
      }
   }

   void clear()
   {
      //TBD
   }
};

struct cPlane
   : public geom::iPlane
{
   cPlaneBase* m_pPlane;

   struct cTempShapeList
   {
      std::list<iGeomImpl*> m_shapes;

      ~cTempShapeList()
      {
         clear();
      }

      void clear()
      {
         for (auto l : m_shapes) {
            l->release();
         }
         m_shapes.clear();
      }
      void push_back(geom::iShape* ps)
      {
         m_shapes.push_back((iGeomImpl*)ps);
      }
      size_t size()
      {
         return m_shapes.size();
      }

      void commit(cGeomTypeDesc& type_desc)
      {
         if (auto size = m_shapes.size()) {

            for (auto& shape : m_shapes) {
               size += shape->holes().size();
            }

            type_desc.m_shapes.reserve(type_desc.m_shapes.size() + size);
            cGeomTypeDesc::cSpatialIndex new_index(size);

            auto add = [this, &new_index, &type_desc](iGeomImpl* shape) {
               geom::cRect bounds = shape->rectangle();
               new_index.add(bounds.m_left, bounds.m_bottom, bounds.m_right, bounds.m_top);
               type_desc.m_shapes.emplace_back(shape->geom_data());
            };

            for (auto& shape : m_shapes) {
               add(shape);
               auto& holes = shape->holes();
               std::ranges::for_each(holes, add);
               holes.clear();
            }

            new_index.finish();
            type_desc.m_index = std::move(new_index);
            clear();
         }
      }
   };

   std::map<geom::ObjectType, cTempShapeList> m_shapes_temp;

   cPlane(cPlaneBase* pPlane)
      : m_pPlane(pPlane)
   {
   }
   ~cPlane()
   {
      commit();
   }

   int id() const override
   {
      return m_pPlane->m_id;
   }

   const char* name() const override
   {
      return m_pPlane->m_name.c_str();
   }

   void add_shape(geom::iShape* ps, geom::ObjectType type) override
   {
      m_shapes_temp[type].push_back(ps);
   }
   void remove_shape(geom::iShape* ps) override
   {
   }
   geom::cShapeIter shapes(const geom::cRect& bounds, geom::ObjectType type, RetrieveOptions opt = RetrieveOptions::shape) const override
   {
      if (auto type_desc = m_pPlane->get_type_desc(type)) {
         geom::iShapeIter* res = nullptr;
         if (type_desc->shapes(&res, bounds, opt)) {
            return geom::cShapeIter(res);
         }
      }
      return geom::cShapeIter();
   }
   geom::cRect bounds() override
   {
      commit();
      return m_pPlane->bounds();
   }
   void commit()
   {
      for (auto& [type, shapes] : m_shapes_temp) {
         if (shapes.size()) {
            shapes.commit(m_pPlane->m_shape_types[type]);
         }
      }
   }
};
