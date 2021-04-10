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
      commit();

      constexpr auto inf = numeric_limits<coord_t>::infinity();
      coord_t minX = inf, minY = inf, maxX = -inf, maxY = -inf;
      for (auto& [type, desc] : m_shape_types) {
         minX = min(minX, desc.m_index.minX());
         minY = min(minY, desc.m_index.minY());
         maxX = max(maxX, desc.m_index.maxX());
         maxY = max(maxY, desc.m_index.maxY());
      }
      return { minX, minY, maxX, maxY };
   }

   void add_shape(cGeomImplBase *ps, ObjectType type)
   {
      m_shape_types[type].m_shapes.emplace_back(ps);
   }

   void commit()
   {
      for (auto&& [type, desc] : m_shape_types) {
         if (auto size = desc.m_shapes.size()) {
            cGeomTypeDesc::cSpatialIndex new_index(size);
            for (auto &shape: desc.m_shapes) {
               auto rectangle = [](const auto* shape) {
                  switch (shape->m_type) {
                     case iPolygon::Type::circle:
                        return ((cCircleImpl*)shape)->rectangle();
                     case iPolygon::Type::segment:
                        return ((cSegmentImpl*)shape)->rectangle();
                     case iPolygon::Type::arc_segment:
                        return ((cArcSegmentImpl*)shape)->rectangle();
                     case iPolygon::Type::rectangle:
                        return ((cRectImpl*)shape)->rectangle();
                     case iPolygon::Type::polyline:
                        return ((cShapeImpl*)shape)->rectangle();
                  }
                  return cRect();
               };
               cRect bounds = rectangle(&*shape);
               new_index.add(bounds.m_left, bounds.m_bottom, bounds.m_right, bounds.m_top);
            }
            new_index.finish();
            desc.m_index = move(new_index);
         }
      }
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

   struct cTempShapeList
   {
      list<iGeomImpl*> m_shapes;

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
      void push_back(iShape* ps)
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
               cRect bounds = shape->rectangle();
               new_index.add(bounds.m_left, bounds.m_bottom, bounds.m_right, bounds.m_top);
               type_desc.m_shapes.emplace_back(shape->geom_data());
            };

            for (auto& shape : m_shapes) {
               add(shape);
               auto& holes = shape->holes();
               ranges::for_each(holes, add);
               holes.clear();
            }

            new_index.finish();
            type_desc.m_index = move(new_index);
            clear();
         }
      }
   };

   map<ObjectType, cTempShapeList> m_shapes_temp;

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

   void add_shape(iShape* ps, ObjectType type) override
   {
      m_shapes_temp[type].push_back(ps);
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
